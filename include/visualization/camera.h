#ifndef VISUALIZATION_CAMERA_H
#define VISUALIZATION_CAMERA_H

#include "gl_headers.h"

namespace visualization {

enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class Camera {
 public:
  Camera(glm::vec3 center = glm::vec3(0.f, 0.f, 0.f),
         glm::vec3 up = glm::vec3(0.f, 0.f, 1.f), float yaw = 0.f,
         float pitch = 0.f);

  glm::mat4 getViewMatrix();

  void processKeyboard(CameraMovement direction, float delta_time);
  void processMouseMovement(float xoffset, float yoffset,
                            bool constrainpitch = true);
  void processMouseScroll(float yoffset);
  void processMouseButton(int button, int action, int mods);

  glm::vec3 getCenter();
  glm::vec3 getEye();

 private:
  glm::vec3 center_;
  glm::vec3 up_;
  glm::vec3 front_;
  glm::vec3 right_;
  glm::vec3 world_up_;

  float yaw_;
  float pitch_;
  float sensitivity_;
  float range_;

  bool permit_update_;

  void updateCameraVectors();
};
}

#endif
