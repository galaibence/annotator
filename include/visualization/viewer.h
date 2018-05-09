#ifndef VISUALIZATION_VIEWER_H
#define VISUALIZATION_VIEWER_H

#include <cstdint>
#include <string>
#include <memory>

#include "annotation/annotator.h"
#include "common/cloud.h"
#include "gl_headers.h"
#include "visualization/camera.h"
#include "visualization/shader.h"

namespace visualization {

struct Buffers {
  uint32_t VAO;
  std::vector<uint32_t> VBOs;
};

class Viewer {
 private:
  GLFWwindow* window_;

  common::Cloud* cloud_;
  Shader shader_;
  Shader rect_shader_;
  Shader comp_shader_;
  Buffers buffers_;
  Buffers rect_buffers_;

  static int width_;
  static int height_;
  float delta_time_;
  float last_time_;

  static void keyCallback(GLFWwindow* window, int key, int scancode, int action,
                          int mode);
  static void scrollCallback(GLFWwindow* window, double xoffset,
                             double yoffset);
  static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action,
                                  int mods);
  static bool keys_[1024];
  static bool first_mouse_;
  static bool left_mouse_pressed_;
  static int last_x_;
  static int last_y_;
  static Camera camera_;

  static std::unique_ptr<annotation::Annotator> annotator_;

  void bindBuffers();
  void update();
  static void getRay(float x, float y, glm::vec3& orig, glm::vec3& dir);

 public:
  Viewer(common::Cloud* cloud = nullptr);

  void init(uint32_t width, uint32_t height);
  void draw();
  void close();
};
}

#endif
