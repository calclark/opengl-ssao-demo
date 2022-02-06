#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

Camera::Camera()
    : _position{0.0, 0.0, 0.0}, _view_dir{0.0, 0.0, -1.0},
      _up_dir{0.0, 1.0, 0.0},
      _right_dir{1.0, 0.0, 0.0}, _pitch{0.0F}, _yaw{-90.0F} {
}

glm::mat4 Camera::transform() const {
    return glm::lookAt(_position, _position + _view_dir, _up_dir);
}

glm::vec3 Camera::position() const {
    return _position;
}

void Camera::rotate(float pitch, float yaw) {
    _pitch = std::clamp(_pitch + pitch, -89.0F, 89.0F);
    _yaw += yaw;
    auto look = glm::vec3{};
    look.x = std::cos(glm::radians(_yaw)) * std::cos(glm::radians(_pitch));
    look.y = std::sin(glm::radians(_pitch));
    look.z = std::sin(glm::radians(_yaw)) * std::cos(glm::radians(_pitch));
    _view_dir = glm::normalize(look);
    _right_dir =
        glm::normalize(glm::cross(_view_dir, glm::vec3{0.0F, 1.0F, 0.0F}));
    _up_dir = glm::normalize(glm::cross(_right_dir, _view_dir));
}

void Camera::move_forward(float delta) {
    _position += _view_dir * delta;
}

void Camera::move_backward(float delta) {
    move_forward(-delta);
}

void Camera::move_up(float delta) {
    _position += _up_dir * delta;
}

void Camera::move_down(float delta) {
    move_up(-delta);
}

void Camera::move_right(float delta) {
    _position += _right_dir * delta;
}

void Camera::move_left(float delta) {
    move_right(-delta);
}
