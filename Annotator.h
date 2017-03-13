#ifndef ANNOTATOR_H
#define ANNOTATOR_H

#include <cstdint>
#include <vector>

#include <QMatrix>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include <QGLViewer/camera.h>

#define CL_HPP_ENABLE_EXCEPTIONS 
#include <CL/cl.hpp>

#include "DataModel.h"

struct SelectionStruct {
	float points[3 * 5 + 1];
};

class Annotator {
private:
	std::shared_ptr<DataModel> model_;

	QRect rectangle_;
	bool selection_started_ = false;
	std::vector<SelectionStruct> selections_;
	std::vector<SelectionStruct> negative_selections_;
	int active_selection_count_ = 0;

	bool cloud_changed_ = true;
	std::vector<uint8_t> selected_points_;

	bool VAO_initied_ = false;

	QOpenGLVertexArrayObject cloud_VAO_;
	QOpenGLBuffer position_VBO_;
	QOpenGLBuffer color_VBO_;
	QOpenGLBuffer selection_VBO_;

	QOpenGLVertexArrayObject rectangle_VAO_;
	QOpenGLBuffer rectangle_VBO_;

	QOpenGLShaderProgram cloud_shader_;
	QOpenGLShaderProgram rectangle_shader_;

	bool initOpenCL();
	void finalizeSelectionOpenCL();
	void finalizeSelectionOpenMP();
	bool opencl_valid_ = false;
	cl::Platform platform_;
	cl::Device device_;
	cl::Context* pContext_;
	cl::CommandQueue* pQueue_;
	cl::Kernel* pKernel_;
	cl::Buffer* pPosition_buffer_;
	// cl::Buffer selected_points_buffer_;
	// cl::Buffer active_selections_buffer_;
	// cl::Buffer active_negative_selections_buffer_;

public:
	Annotator();

	void setModel(std::shared_ptr<DataModel> model);

	void startSelection(SelectionStruct selection);
	void startNegativeSelection(SelectionStruct selection);
	void stopSelection(SelectionStruct selection);
	void stopNegativeSelection(SelectionStruct selection);

	void setRectangleTopLeft(QPoint point);
	void setRectangleBottomRight(QPoint point);
	void triggerSelectionStart();
	void triggerSelectionEnd();
	
	bool inSelection();

	SelectionStruct getCurrentSelection(qglviewer::Camera*);

	bool initShaders();
	void initDraw();
	void draw(QMatrix4x4& mvp, uint32_t width, uint32_t height);
	void finalizeSelection();

};

#endif
