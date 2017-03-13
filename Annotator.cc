#include "Annotator.h"

#include <fstream>
#include <iostream>

#define CL_HPP_ENABLE_EXCEPTIONS 
#include <CL/cl.hpp>

#include "StopWatch.h"

#include <Eigen/Dense>
#include <QString>
#include <QGLViewer/vec.h>
#include <QGLViewer/camera.h>

Annotator::Annotator() :rectangle_{QPoint{0, 0}, QPoint{0, 0}} {}

void Annotator::setModel(std::shared_ptr<DataModel> model) {
	model_ = model;
}

bool Annotator::initShaders() {
	
	if (!cloud_shader_.addShaderFromSourceFile(QOpenGLShader::Vertex, {"./annotator.vert"})) {
		qWarning() << cloud_shader_.log();
		return false;
	}
	if (!cloud_shader_.addShaderFromSourceFile(QOpenGLShader::Fragment, "./annotator.frag")) {
		qWarning() << cloud_shader_.log();
		return false;
	}
	if (!cloud_shader_.link()) {
		qWarning() << "Could not link shader program:" << cloud_shader_.log();
		return false;
	}

	if (!rectangle_shader_.addShaderFromSourceFile(QOpenGLShader::Vertex, "./screen.vert")) {
		qWarning() << rectangle_shader_.log();
		return false;
	}

	if (!rectangle_shader_.addShaderFromSourceFile(QOpenGLShader::Fragment, "./screen.frag")) {
		qWarning() << rectangle_shader_.log();
		return false;
	}

	if (!rectangle_shader_.link()) {
		qWarning() << "Could not link shader program:" << rectangle_shader_.log();
		return false;
	}
	return true;
}

void Annotator::initDraw() {

	opencl_valid_ = initOpenCL();

	size_t size = model_->spCloud->size();

	float* position = new float[size * 3];
	uint8_t* color = new uint8_t[size * 3];
	selected_points_ = std::vector<uint8_t>(size);

	for (size_t i = 0; i < size; i++) {
		position[i * 3 + 0] = model_->spCloud->points[i].x;
		position[i * 3 + 1] = model_->spCloud->points[i].y;
		position[i * 3 + 2] = model_->spCloud->points[i].z;
		color[i * 3 + 0] = model_->spCloud->points[i].r;
		color[i * 3 + 1] = model_->spCloud->points[i].g;
		color[i * 3 + 2] = model_->spCloud->points[i].b;
	}

	if (!VAO_initied_) {
		cloud_VAO_.create();
		position_VBO_.create();
		color_VBO_.create();
		selection_VBO_.create();

		VAO_initied_ = true;
	}

	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	cloud_VAO_.bind();

	position_VBO_.bind();
	position_VBO_.allocate(position, size * 3 * sizeof(float));
	f->glEnableVertexAttribArray(0);
	f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	position_VBO_.release();

	color_VBO_.bind();
	color_VBO_.allocate(color, size * 3 * sizeof(uint8_t));
	f->glEnableVertexAttribArray(1);
	f->glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_FALSE, 3 * sizeof(uint8_t), 0);

	selection_VBO_.bind();
	selection_VBO_.allocate(selected_points_.data(), size * 1 * sizeof(uint8_t));
	f->glEnableVertexAttribArray(2);
	f->glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(uint8_t), 0);

	cloud_VAO_.release();

	delete[] position;
	delete[] color;
}

void Annotator::draw(QMatrix4x4& mvp, uint32_t width, uint32_t height) {

	cloud_shader_.bind();

	cloud_shader_.setUniformValueArray("mvp", &mvp, 1);

	float* selections = new float[selections_.size() * 16];
	for (int i = 0; i < selections_.size(); i++) {
		for (int j = 0; j < 16; j++) {
			selections[i * 16 + j] = selections_[i].points[j];
		}
	}
	cloud_shader_.setUniformValueArray("active_selection", selections, selections_.size() * 16, 1);

	float* negative_selections = new float[negative_selections_.size() * 16];
	for (int i = 0; i < negative_selections_.size(); i++) {
		for (int j = 0; j < 16; j++) {
			negative_selections[i * 16 + j] = negative_selections_[i].points[j];
		}
	}
	cloud_shader_.setUniformValueArray("active_negative_selection", negative_selections, negative_selections_.size() * 16, 1);

	int number_of_active_selections = (int)selections_.size();
	cloud_shader_.setUniformValueArray("number_of_active_selections", &number_of_active_selections, 1);
	int number_of_active_negative_selections = (int)negative_selections_.size();
	cloud_shader_.setUniformValueArray("number_of_active_negative_selections", &number_of_active_negative_selections, 1);

	delete[] selections;
	delete[] negative_selections;

	cloud_VAO_.bind();
	glDrawArrays(GL_POINTS, 0, model_->spCloud->size());
	cloud_VAO_.release();

	cloud_shader_.release();

	if (selection_started_) {
		QVector4D Ps[4];
		int rec_min_x = std::min(rectangle_.topLeft().x(), rectangle_.bottomRight().x());
		int rec_max_x = std::max(rectangle_.topLeft().x(), rectangle_.bottomRight().x());
		int rec_min_y = std::min(rectangle_.topLeft().y(), rectangle_.bottomRight().y());
		int rec_max_y = std::max(rectangle_.topLeft().y(), rectangle_.bottomRight().y());
		Ps[0] = QVector4D(rec_min_x, rec_min_y, 0.0f, 1.0f);
		Ps[1] = QVector4D(rec_max_x, rec_min_y, 0.0f, 1.0f);
		Ps[2] = QVector4D(rec_max_x, rec_max_y, 0.0f, 1.0f);
		Ps[3] = QVector4D(rec_min_x, rec_max_y, 0.0f, 1.0f);

		float* lines = new float[16];
		for (auto k = 0; k < 4; ++k) {
			lines[k * 4 + 0] = Ps[k][0];
			lines[k * 4 + 1] = Ps[k][1];
			lines[k * 4 + 2] = Ps[(k + 1) % 4][0];
			lines[k * 4 + 3] = Ps[(k + 1) % 4][1];
		}

		rectangle_VAO_.bind();
		rectangle_VBO_.create();
		rectangle_VBO_.bind();
		rectangle_VBO_.allocate(lines, sizeof(float) * 16);
		delete[] lines;
		rectangle_VAO_.release();

		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

		rectangle_shader_.bind();
		rectangle_shader_.setUniformValueArray("width", &width, 1);
		rectangle_shader_.setUniformValueArray("height", &height, 1);
		
		rectangle_VAO_.bind();
		glDrawArrays(GL_LINES, 0, 8);
		rectangle_VAO_.release();

		rectangle_shader_.release();
	}
}

void Annotator::finalizeSelection() {
	if (false && opencl_valid_)
		finalizeSelectionOpenCL();
	else
		finalizeSelectionOpenMP();
	
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	cloud_VAO_.bind();
	selection_VBO_.bind();
	selection_VBO_.allocate(selected_points_.data(), selected_points_.size() * 1 * sizeof(uint8_t));
	f->glEnableVertexAttribArray(2);
	f->glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(uint8_t), 0);
	cloud_VAO_.release();

	selections_.clear();
	selections_.resize(0);
	negative_selections_.clear();
	negative_selections_.resize(0);
	active_selection_count_ = 0;
}

void Annotator::finalizeSelectionOpenMP() {

	StopWatch sw;
	sw.start();

	#pragma omp parallel for
	for (size_t k = 0; k < model_->spCloud->size(); k++) {
		Eigen::Vector3f vertex(model_->spCloud->points[k].x,
							   model_->spCloud->points[k].y,
							   model_->spCloud->points[k].z);

		bool active_selected = false;
		int last_selection_if_selected = -1;
		for (size_t i = 0; i < selections_.size(); i++) {
			Eigen::Vector3f orig = Eigen::Vector3f(
				selections_[i].points[0 * 3 + 0],
				selections_[i].points[0 * 3 + 1],
				selections_[i].points[0 * 3 + 2]);
			Eigen::Vector3f dir0 = Eigen::Vector3f(
				selections_[i].points[1 * 3 + 0],
				selections_[i].points[1 * 3 + 1],
				selections_[i].points[1 * 3 + 2]);
			Eigen::Vector3f dir1 = Eigen::Vector3f(
				selections_[i].points[2 * 3 + 0],
				selections_[i].points[2 * 3 + 1],
				selections_[i].points[2 * 3 + 2]);
			Eigen::Vector3f dir2 = Eigen::Vector3f(
				selections_[i].points[3 * 3 + 0],
				selections_[i].points[3 * 3 + 1],
				selections_[i].points[3 * 3 + 2]);
			Eigen::Vector3f dir3 = Eigen::Vector3f(
				selections_[i].points[4 * 3 + 0],
				selections_[i].points[4 * 3 + 1],
				selections_[i].points[4 * 3 + 2]);

			Eigen::Vector3f N0 = dir0.normalized().cross(dir1.normalized()).normalized();
			Eigen::Vector3f N1 = dir1.normalized().cross(dir2.normalized()).normalized();
			Eigen::Vector3f N2 = dir2.normalized().cross(dir3.normalized()).normalized();
			Eigen::Vector3f N3 = dir3.normalized().cross(dir0.normalized()).normalized();

			if ((N0.dot((vertex - orig)) > 0 && N1.dot((vertex - orig)) > 0 && N2.dot((vertex - orig)) > 0 && N3.dot((vertex - orig)) > 0)) {
				active_selected = true;
				last_selection_if_selected = int(selections_[i].points[i * 16 + 15]);
			}
		}

		int last_negative_selection_if_selected = -1;
		for (size_t i = 0; i < negative_selections_.size(); i++) {
			Eigen::Vector3f orig = Eigen::Vector3f(
				negative_selections_[i].points[0 * 3 + 0],
				negative_selections_[i].points[0 * 3 + 1],
				negative_selections_[i].points[0 * 3 + 2]);
			Eigen::Vector3f dir0 = Eigen::Vector3f(
				negative_selections_[i].points[1 * 3 + 0],
				negative_selections_[i].points[1 * 3 + 1],
				negative_selections_[i].points[1 * 3 + 2]);
			Eigen::Vector3f dir1 = Eigen::Vector3f(
				negative_selections_[i].points[2 * 3 + 0],
				negative_selections_[i].points[2 * 3 + 1],
				negative_selections_[i].points[2 * 3 + 2]);
			Eigen::Vector3f dir2 = Eigen::Vector3f(
				negative_selections_[i].points[3 * 3 + 0],
				negative_selections_[i].points[3 * 3 + 1],
				negative_selections_[i].points[3 * 3 + 2]);
			Eigen::Vector3f dir3 = Eigen::Vector3f(
				negative_selections_[i].points[4 * 3 + 0],
				negative_selections_[i].points[4 * 3 + 1],
				negative_selections_[i].points[4 * 3 + 2]);

			Eigen::Vector3f N0 = dir0.normalized().cross(dir1.normalized()).normalized();
			Eigen::Vector3f N1 = dir1.normalized().cross(dir2.normalized()).normalized();
			Eigen::Vector3f N2 = dir2.normalized().cross(dir3.normalized()).normalized();
			Eigen::Vector3f N3 = dir3.normalized().cross(dir0.normalized()).normalized();

			if ((N0.dot((vertex - orig)) > 0 && N1.dot((vertex - orig)) > 0 && N2.dot((vertex - orig)) > 0 && N3.dot((vertex - orig)) > 0)) {
				last_negative_selection_if_selected = int(negative_selections_[i].points[i * 16 + 15]);
			}
		}

		if (active_selected && last_selection_if_selected > last_negative_selection_if_selected) {
			selected_points_[k] = 255;
		}
	}
	sw.end();
	std::cout << "OpenMP duration: " << sw.duration() << " ms." << std::endl;
}

void Annotator::startSelection(SelectionStruct selection) {
	active_selection_count_++;
	selection.points[15] = active_selection_count_;
	selections_.push_back(selection);
}

void Annotator::startNegativeSelection(SelectionStruct selection) {
	active_selection_count_++;
	selection.points[15] = active_selection_count_;
	negative_selections_.push_back(selection);
}

void Annotator::stopSelection(SelectionStruct selection) {
	selection.points[15] = active_selection_count_;
	selections_[selections_.size() - 1] = selection;
}

void Annotator::stopNegativeSelection(SelectionStruct selection) {
	selection.points[15] = active_selection_count_;
	negative_selections_[negative_selections_.size() - 1] = selection;
}

void Annotator::setRectangleTopLeft(QPoint point) {
	rectangle_.setTopLeft(point);
}

void Annotator::setRectangleBottomRight(QPoint point) {
	rectangle_.setBottomRight(point);
}

void Annotator::triggerSelectionStart() {
	selection_started_ = true;
}
void Annotator::triggerSelectionEnd() {
	selection_started_ = false;
}

bool Annotator::inSelection() {
	return selection_started_;
}

SelectionStruct Annotator::getCurrentSelection(qglviewer::Camera* camera) {
	QVector4D Ps[4];
	int rec_min_x = std::min(rectangle_.topLeft().x(), rectangle_.bottomRight().x());
	int rec_max_x = std::max(rectangle_.topLeft().x(), rectangle_.bottomRight().x());
	int rec_min_y = std::min(rectangle_.topLeft().y(), rectangle_.bottomRight().y());
	int rec_max_y = std::max(rectangle_.topLeft().y(), rectangle_.bottomRight().y());
	Ps[0] = QVector4D(rec_min_x, rec_min_y, 0.0f, 1.0f);
	Ps[1] = QVector4D(rec_max_x, rec_min_y, 0.0f, 1.0f);
	Ps[2] = QVector4D(rec_max_x, rec_max_y, 0.0f, 1.0f);
	Ps[3] = QVector4D(rec_min_x, rec_max_y, 0.0f, 1.0f);

	qglviewer::Vec dir[4], orig;
	camera->convertClickToLine({rec_min_x, rec_min_y}, orig, dir[0]);
	camera->convertClickToLine({rec_max_x, rec_min_y}, orig, dir[1]);
	camera->convertClickToLine({rec_max_x, rec_max_y}, orig, dir[2]);
	camera->convertClickToLine({rec_min_x, rec_max_y}, orig, dir[3]);

	SelectionStruct selection;
	selection.points[0 * 3 + 0] = orig.x;
	selection.points[0 * 3 + 1] = orig.y;
	selection.points[0 * 3 + 2] = orig.z;
	for (int i = 0; i < 4; i++) {
		selection.points[(i + 1) * 3 + 0] = dir[i].x;
		selection.points[(i + 1) * 3 + 1] = dir[i].y;
		selection.points[(i + 1) * 3 + 2] = dir[i].z;
	}
	return selection;
}

std::string get_file_contents(std::string filename) {
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in) {
	  
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }

  return "";
}


bool Annotator::initOpenCL() {
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0) return false;

	std::cout << "Number of platforms: " << all_platforms.size() << std::endl;
	platform_ = all_platforms[0];
	std::cout << "Using platform: " << platform_.getInfo<CL_PLATFORM_NAME>() << "\n";

	std::vector<cl::Device> gpu_devices, cpu_devices;
	platform_.getDevices(CL_DEVICE_TYPE_GPU, &gpu_devices);
	platform_.getDevices(CL_DEVICE_TYPE_CPU, &cpu_devices);
	if ((gpu_devices.size() + cpu_devices.size()) == 0) return false;

	std::cout << "Number of devices: " << gpu_devices.size() + cpu_devices.size() << std::endl;
	if (gpu_devices.size() > 0) 
		device_ = gpu_devices[0];
	else
		device_ = cpu_devices[0];
	std::cout<< "Using device: " << device_.getInfo<CL_DEVICE_NAME>() << "\n";

	pContext_ = new cl::Context({device_});

	std::string kernel_code = get_file_contents("./annotator.cl");
	cl::Program::Sources sources;
	sources.push_back({kernel_code.c_str(), kernel_code.length()});

	cl::Program program = cl::Program(*pContext_, sources);
	uint32_t error = program.build({device_});
	if (error != CL_SUCCESS) return false;	

	pKernel_ = new cl::Kernel(program, "select_points");
	pQueue_ = new cl::CommandQueue(*pContext_, device_);

	std::cout << "OpenCL successfully initialized" << std::endl;

	return true;
}

void Annotator::finalizeSelectionOpenCL() {

	StopWatch sw;
	sw.start();

	pPosition_buffer_ = new cl::Buffer(*pContext_, CL_MEM_READ_ONLY, sizeof(float) * model_->spCloud->points.size() * 3);
	float* position = new float[model_->spCloud->size() * 3];
	for (size_t i = 0; i < model_->spCloud->size(); i++) {
		position[i * 3 + 0] = model_->spCloud->points[i].x;
		position[i * 3 + 1] = model_->spCloud->points[i].y;
		position[i * 3 + 2] = model_->spCloud->points[i].z;
	}
  	pQueue_->enqueueWriteBuffer(*pPosition_buffer_, CL_TRUE, 0, sizeof(float) *model_->spCloud->points.size() * 3, position);

	  sw.end();
	  std::cout << sw.duration() << " ms." << std::endl;
	  sw.start();

	cl::Buffer active_selections_buffer_(*pContext_, CL_MEM_READ_ONLY, sizeof(float) * selections_.size() * 16);
	float* selections = new float[selections_.size() * 16];
	for (int i = 0; i < selections_.size(); i++) {
		for (int j = 0; j < 16; j++) {
			selections[i * 16 + j] = selections_[i].points[j];
		}
	}
	pQueue_->enqueueWriteBuffer(active_selections_buffer_, CL_TRUE, 0, sizeof(float) * selections_.size() * 16, selections);

		sw.end();
	  std::cout << sw.duration() << " ms." << std::endl;
	  sw.start();

	cl::Buffer active_negative_selections_buffer_(*pContext_, CL_MEM_READ_ONLY, sizeof(float) * negative_selections_.size() * 16);
	float* negative_selections = new float[negative_selections_.size() * 16];
	for (int i = 0; i < negative_selections_.size(); i++) {
		for (int j = 0; j < 16; j++) {
			negative_selections[i * 16 + j] = negative_selections_[i].points[j];
		}
	}
	pQueue_->enqueueWriteBuffer(active_negative_selections_buffer_, CL_TRUE, 0, sizeof(float) * negative_selections_.size() * 16, negative_selections);

	sw.end();
	  std::cout << sw.duration() << " ms." << std::endl;
	  sw.start();

	cl::Buffer selected_points_buffer_(*pContext_, CL_MEM_READ_WRITE, sizeof(uint8_t) * model_->spCloud->size());
	pQueue_->enqueueWriteBuffer(selected_points_buffer_, CL_TRUE, 0, sizeof(uint8_t) * model_->spCloud->size(), selected_points_.data());  	

	sw.end();
	  std::cout << sw.duration() << " ms." << std::endl;
	  sw.start();

	pKernel_->setArg(0, *pPosition_buffer_);
	pKernel_->setArg(1, active_selections_buffer_);
	pKernel_->setArg(2, active_negative_selections_buffer_);
	pKernel_->setArg(3, (int)selections_.size());
	pKernel_->setArg(4, (int)negative_selections_.size());
	pKernel_->setArg(5, selected_points_buffer_);

	pQueue_->enqueueNDRangeKernel(*pKernel_, cl::NullRange, cl::NDRange(model_->spCloud->points.size()), cl::NullRange);
	pQueue_->finish();
	
	sw.end();
	std::cout << sw.duration() << " ms." << std::endl;
	sw.start();
	
	pQueue_->enqueueReadBuffer(selected_points_buffer_, CL_TRUE, 0, sizeof(uint8_t) * model_->spCloud->size(), selected_points_.data());

	sw.end();
	std::cout << sw.duration() << " ms." << std::endl;
	sw.start();

	delete pPosition_buffer_;
	delete[] position;
	delete[] selections;
	delete[] negative_selections;
}