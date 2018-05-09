#include "visualization/viewer.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>

#include "annotation/annotator.h"
#include "annotation/selection.h"
#include "common/attributes.h"
#include "common/cloud.h"
#include "gl_headers.h"
#include "visualization/camera.h"
#include "visualization/shader.h"

namespace visualization {

bool Viewer::keys_[1024];
bool Viewer::first_mouse_ = true;
bool Viewer::left_mouse_pressed_ = false;
int Viewer::last_x_;
int Viewer::last_y_;
int Viewer::width_;
int Viewer::height_;

Camera Viewer::camera_;
std::unique_ptr<annotation::Annotator> Viewer::annotator_{
    new annotation::Annotator()};

Viewer::Viewer(common::Cloud* cloud)
    : window_{nullptr}, cloud_{cloud}, last_time_{0.f} {}

void Viewer::bindBuffers() {
  common::FloatAttributeMap float_attributes = cloud_->getFloatAttributes();
  common::UintAttributeMap uint_attributes = cloud_->getUintAttributes();

  size_t attribute_count = float_attributes.size() + uint_attributes.size();
  buffers_.VBOs.resize(attribute_count);

  glGenVertexArrays(1, &buffers_.VAO);
  // glBindVertexArray(buffers_.VAO);

  std::map<std::string, int> shader_attrib_locations;
  shader_attrib_locations.insert(std::pair<std::string, int>("position", 0));
  shader_attrib_locations.insert(std::pair<std::string, int>("color", 1));
  shader_attrib_locations.insert(std::pair<std::string, int>("validity", 2));

  size_t VBO_idx = 0;
  for (auto it = float_attributes.begin(); it != float_attributes.end(); it++) {
    glGenBuffers(1, &buffers_.VBOs[VBO_idx]);
    // glBindBuffer(GL_ARRAY_BUFFER, buffers_.VBOs[VBO_idx++]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers_.VBOs[VBO_idx]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * it->second->size(),
    //              it->second->data(), GL_STATIC_DRAW);
    glBufferData(GL_SHADER_STORAGE_BUFFER, it->second->size() * sizeof(float),
                 nullptr, GL_STATIC_DRAW);
    float* data = (float*)glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, it->second->size() * sizeof(float),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(data, it->second->data(), it->second->size() * sizeof(float));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    auto attrib_it = shader_attrib_locations.find(it->second->attribute);
    if (attrib_it != shader_attrib_locations.end()) {
      glBindVertexArray(buffers_.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, buffers_.VBOs[VBO_idx]);
      glVertexAttribPointer(
          attrib_it->second, it->second->componentSize(), GL_FLOAT, GL_FALSE,
          it->second->componentSize() * sizeof(float), (void*)0);
      glEnableVertexAttribArray(attrib_it->second);
      glBindVertexArray(0);
    }

    VBO_idx++;
  }

  for (auto it = uint_attributes.begin(); it != uint_attributes.end(); it++) {
    glGenBuffers(1, &buffers_.VBOs[VBO_idx]);
    // glBindBuffer(GL_ARRAY_BUFFER, buffers_.VBOs[VBO_idx++]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers_.VBOs[VBO_idx]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(uint32_t) * it->second->size(),
    //              it->second->data(), GL_STATIC_DRAW);
    glBufferData(GL_SHADER_STORAGE_BUFFER, it->second->size() * sizeof(uint32_t),
                 nullptr, GL_STATIC_DRAW);
    uint32_t* data = (uint32_t*)glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, it->second->size() * sizeof(uint32_t),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(data, it->second->data(), it->second->size() * sizeof(uint32_t));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    auto attrib_it = shader_attrib_locations.find(it->second->attribute);
    if (attrib_it != shader_attrib_locations.end()) {
      glBindVertexArray(buffers_.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, buffers_.VBOs[VBO_idx]);
      glVertexAttribPointer(
          attrib_it->second, it->second->componentSize(), GL_UNSIGNED_INT,
          GL_FALSE, it->second->componentSize() * sizeof(uint32_t), (void*)0);
      glEnableVertexAttribArray(attrib_it->second);
      glBindVertexArray(0);
    }

    VBO_idx++;
  }

  glBindVertexArray(0);

  rect_buffers_.VBOs.resize(1);
  glGenVertexArrays(1, &rect_buffers_.VAO);
  glBindVertexArray(rect_buffers_.VAO);
  glGenBuffers(1, &rect_buffers_.VBOs[0]);
  glBindVertexArray(0);
}

void Viewer::draw() {
  float current_time;
  while (!glfwWindowShouldClose(window_)) {
    current_time = glfwGetTime();
    delta_time_ = current_time - last_time_;
    last_time_ = current_time;

    glfwPollEvents();
    update();

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffers_.VBOs[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffers_.VBOs[2]);
    shader_.use();

    glm::mat4 view = camera_.getViewMatrix();
    glm::mat4 projection =
        glm::perspective(45.f, (float)width_ / (float)height_, 0.01f, 1000.0f);
    glm::mat4 model = glm::mat4();

    int modelLoc = glGetUniformLocation(shader_.getProgram(), "model");
    int viewLoc = glGetUniformLocation(shader_.getProgram(), "view");
    int projLoc = glGetUniformLocation(shader_.getProgram(), "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform1i(glGetUniformLocation(shader_.getProgram(),
                                     "number_of_active_selections"),
                annotator_->numberOfActiveSelections());

    std::vector<float> selections_float_array;
    selections_float_array.resize(16 * annotator_->numberOfActiveSelections());
    annotation::Selection* active_selections =
        annotator_->getActiveSelectionFloatArray();
    for (int i = 0; i < annotator_->numberOfActiveSelections(); i++) {
      for (int j = 0; j < 16; j++) {
        selections_float_array[i * 16 + j] = active_selections[i].p[j];
      }
    }

    glUniform1fv(glGetUniformLocation(shader_.getProgram(), "active_selection"),
                 16 * annotator_->numberOfActiveSelections(),
                 selections_float_array.data());
    glUniform1i(glGetUniformLocation(shader_.getProgram(),
                                     "number_of_active_negative_selections"),
                annotator_->numberOfActiveNegativeSelections());

    std::vector<float> negative_selections_float_array;
    negative_selections_float_array.resize(
        16 * annotator_->numberOfActiveNegativeSelections());
    annotation::Selection* active_negative_selections =
        annotator_->getActiveNegativeSelectionFloatArray();
    for (int i = 0; i < annotator_->numberOfActiveNegativeSelections(); i++) {
      for (int j = 0; j < 16; j++) {
        negative_selections_float_array[i * 16 + j] =
            active_negative_selections[i].p[j];
      }
    }

    glUniform1fv(
        glGetUniformLocation(shader_.getProgram(), "active_negative_selection"),
        16 * annotator_->numberOfActiveNegativeSelections(),
        negative_selections_float_array.data());

    glBindVertexArray(buffers_.VAO);
    glDrawArrays(GL_POINTS, 0, cloud_->size());
    glBindVertexArray(0);

    if (annotator_->inSelection()) {
      rect_shader_.use();

      float topLeftX, topLeftY, bottomRightX, bottomRightY;
      annotator_->getRectangle(topLeftX, topLeftY, bottomRightX, bottomRightY);

      float rect_data[] = {
          topLeftX,     topLeftY,     topLeftX,     bottomRightY,
          topLeftX,     bottomRightY, bottomRightX, bottomRightY,
          bottomRightX, bottomRightY, bottomRightX, topLeftY,
          bottomRightX, topLeftY,     topLeftX,     topLeftY};

      glUniform1i(glGetUniformLocation(rect_shader_.getProgram(), "width"),
                  width_);
      glUniform1i(glGetUniformLocation(rect_shader_.getProgram(), "height"),
                  height_);

      glBindVertexArray(rect_buffers_.VAO);
      glBindBuffer(GL_ARRAY_BUFFER, rect_buffers_.VBOs[0]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, rect_data,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                            (void*)0);
      glEnableVertexAttribArray(0);

      glDrawArrays(GL_LINES, 0, 8);
      glBindVertexArray(0);
    }

    glfwSwapBuffers(window_);
  }
}

void Viewer::init(uint32_t width, uint32_t height) {
  height_ = height;
  width_ = width;

  if (!glfwInit()) {
    std::cout << "Cannot initialize GLFW!" << std::endl;
    exit(-1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  window_ = glfwCreateWindow(width_, height_, "Cloud Viewer", nullptr, nullptr);
  if (window_ == nullptr) {
    std::cout << "Cannot create GLFW window!" << std::endl;
    exit(-1);
  }

  glfwMakeContextCurrent(window_);

  glfwSetKeyCallback(window_, keyCallback);
  glfwSetCursorPosCallback(window_, mouseCallback);
  glfwSetScrollCallback(window_, scrollCallback);
  glfwSetMouseButtonCallback(window_, mouseButtonCallback);

  glewExperimental = GL_TRUE;
  if (GLEW_OK != glewInit()) {
    std::cout << "Cannot initialize GLEW" << std::endl;
    exit(-1);
  }

  glViewport(0, 0, width_, height_);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);

  if (cloud_ != nullptr) {
    bindBuffers();
  }

  shader_.loadShader(GL_VERTEX_SHADER, "resources/anno.vert");
  shader_.loadShader(GL_GEOMETRY_SHADER, "resources/anno.geom");
  shader_.loadShader(GL_FRAGMENT_SHADER, "resources/anno.frag");
  shader_.compileShaders();

  rect_shader_.loadShader(GL_VERTEX_SHADER, "resources/rect.vert");
  rect_shader_.loadShader(GL_FRAGMENT_SHADER, "resources/rect.frag");
  rect_shader_.compileShaders();

  comp_shader_.loadShader(GL_COMPUTE_SHADER, "resources/anno.comp");
  comp_shader_.compileShaders();
}

void Viewer::close() {
  glDeleteVertexArrays(1, &buffers_.VAO);
  glDeleteVertexArrays(1, &rect_buffers_.VAO);
  for (auto& VBO : buffers_.VBOs) glDeleteBuffers(1, &VBO);
  for (auto& VBO : rect_buffers_.VBOs) glDeleteBuffers(1, &VBO);
  glfwTerminate();
}

void Viewer::update() {
  if (keys_[GLFW_KEY_F]) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffers_.VBOs[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffers_.VBOs[2]);
    comp_shader_.use();

    glUniform1i(glGetUniformLocation(comp_shader_.getProgram(),
                                     "number_of_active_selections"),
                annotator_->numberOfActiveSelections());

    std::vector<float> selections_float_array;
    selections_float_array.resize(16 * annotator_->numberOfActiveSelections());
    annotation::Selection* active_selections =
        annotator_->getActiveSelectionFloatArray();
    for (int i = 0; i < annotator_->numberOfActiveSelections(); i++) {
      for (int j = 0; j < 16; j++) {
        selections_float_array[i * 16 + j] = active_selections[i].p[j];
      }
    }

    glUniform1fv(
        glGetUniformLocation(comp_shader_.getProgram(), "active_selection"),
        16 * annotator_->numberOfActiveSelections(),
        selections_float_array.data());
    glUniform1i(glGetUniformLocation(comp_shader_.getProgram(),
                                     "number_of_active_negative_selections"),
                annotator_->numberOfActiveNegativeSelections());

    std::vector<float> negative_selections_float_array;
    negative_selections_float_array.resize(
        16 * annotator_->numberOfActiveNegativeSelections());
    annotation::Selection* active_negative_selections =
        annotator_->getActiveNegativeSelectionFloatArray();
    for (int i = 0; i < annotator_->numberOfActiveNegativeSelections(); i++) {
      for (int j = 0; j < 16; j++) {
        negative_selections_float_array[i * 16 + j] =
            active_negative_selections[i].p[j];
      }
    }

    glUniform1fv(glGetUniformLocation(comp_shader_.getProgram(),
                                      "active_negative_selection"),
                 16 * annotator_->numberOfActiveNegativeSelections(),
                 negative_selections_float_array.data());

    glDispatchCompute(cloud_->size(), 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
                    GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    annotator_->clearSelection();
  }
}

void Viewer::keyCallback(GLFWwindow* window, int key, int scancode, int action,
                         int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
  if (key >= 0 && key < 1024) {
    if (action == GLFW_PRESS)
      keys_[key] = true;
    else if (action == GLFW_RELEASE)
      keys_[key] = false;
  }
}

void Viewer::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
  if (first_mouse_) {
    last_x_ = xpos;
    last_y_ = ypos;
    first_mouse_ = false;
  }

  float xoffset = xpos - last_x_;
  float yoffset = last_y_ - ypos;

  last_x_ = xpos;
  last_y_ = ypos;

  if (!annotator_->inSelection()) {
    camera_.processMouseMovement(xoffset, yoffset);
  } else if (annotator_->inSelection()) {
    if (left_mouse_pressed_ && GLFW_MOD_SHIFT) {
      annotator_->triggerSelectionStart();
      annotator_->changeBottomRight(last_x_, last_y_);
      annotator_->stopSelection(annotator_->getCurrentSelection(getRay));
    } else if (left_mouse_pressed_ && GLFW_MOD_CONTROL) {
      annotator_->triggerSelectionStart();
      annotator_->changeBottomRight(last_x_, last_y_);
      annotator_->stopNegativeSelection(
          annotator_->getCurrentSelection(getRay));
    }
  }
}

void Viewer::scrollCallback(GLFWwindow* window, double xoffset,
                            double yoffset) {
  camera_.processMouseScroll(yoffset);
}

void Viewer::mouseButtonCallback(GLFWwindow* window, int button, int action,
                                 int mods) {
  camera_.processMouseButton(button, action, mods);

  left_mouse_pressed_ = button == GLFW_MOUSE_BUTTON_LEFT;

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
      (mods & GLFW_MOD_SHIFT)) {
    annotator_->triggerSelectionStart();
    annotator_->changeTopLeft(last_x_, last_y_);
    annotator_->changeBottomRight(last_x_, last_y_);
    annotator_->startSelection(annotator_->getCurrentSelection(getRay));
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
             (mods & GLFW_MOD_CONTROL)) {
    annotator_->triggerSelectionStart();
    annotator_->changeTopLeft(last_x_, last_y_);
    annotator_->changeBottomRight(last_x_, last_y_);
    annotator_->startNegativeSelection(annotator_->getCurrentSelection(getRay));
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE &&
             (mods & GLFW_MOD_SHIFT)) {
    annotator_->triggerSelectionEnd();
    annotator_->changeBottomRight(last_x_, last_y_);
    annotator_->stopSelection(annotator_->getCurrentSelection(getRay));

    annotator_->startSelection(annotator_->getCurrentSelection(getRay));
    annotator_->stopSelection(annotator_->getCurrentSelection(getRay));
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE &&
             (mods & GLFW_MOD_CONTROL)) {
    annotator_->triggerSelectionEnd();
    annotator_->changeBottomRight(last_x_, last_y_);
    annotator_->stopNegativeSelection(annotator_->getCurrentSelection(getRay));
  }
}

void Viewer::getRay(float x, float y, glm::vec3& orig, glm::vec3& dir) {
  glm::vec4 ray_clip = glm::vec4{(2.0f * x) / width_ - 1.0f,
                                 1.0f - (2.0f * y) / height_, -1.0, 1.0};

  glm::mat4 view = camera_.getViewMatrix();
  glm::mat4 projection =
      glm::perspective(45.f, (float)width_ / (float)height_, 0.1f, 1000.f);

  glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
  ray_eye = glm::vec4{ray_eye.x, ray_eye.y, -1.0, 0.0};
  ray_eye = inverse(view) * ray_eye;

  glm::vec3 ray_wor = glm::vec3{ray_eye.x, ray_eye.y, ray_eye.z};

  dir = glm::normalize(ray_wor);
  orig = camera_.getEye();
}
}
