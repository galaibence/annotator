#include "visualization/camera.h"

#include <iostream>

#include "gl_headers.h"

namespace visualization {

void Camera::updateCameraVectors() {
  glm::vec3 front;
  front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front.y = -sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front.z = sin(glm::radians(pitch_));

  front_ = glm::normalize(front);
  right_ = glm::normalize(glm::cross(front_, world_up_));
  up_ = glm::normalize(glm::cross(right_, front_));
}

Camera::Camera(glm::vec3 center, glm::vec3 up, float yaw, float pitch)
    : center_{center},
      front_{0.f, 1.f, 0.f},
      world_up_{up},
      yaw_{yaw},
      pitch_{pitch},
      sensitivity_{0.25f},
      range_{10.f},
      permit_update_{false} {
  updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
  return glm::lookAt(center_ - front_ * range_, center_, up_);
}

void Camera::processKeyboard(CameraMovement direction, float delta_time) {}

void Camera::processMouseMovement(float xoffset, float yoffset,
                                  bool constrainpitch) {
  if (permit_update_) {
    xoffset *= sensitivity_;
    yoffset *= sensitivity_;

    yaw_ += xoffset;
    pitch_ += yoffset;

    if (constrainpitch) {
      if (pitch_ > 89.0f) pitch_ = 89.0f;
      if (pitch_ < -89.0f) pitch_ = -89.0f;
    }

    updateCameraVectors();
  }
}

void Camera::processMouseScroll(float yoffset) {
  range_ += yoffset * 0.1f;  // SCROLL SENSITIVITY
  if (range_ < 0.f) range_ = 0.f;
}

void Camera::processMouseButton(int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    permit_update_ = true;
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    permit_update_ = false;
}

glm::vec3 Camera::getCenter() { return center_; }

glm::vec3 Camera::getEye() { return center_ - range_ * front_; }
}
