#pragma once

#include <glm/glm.hpp>

// A camera is a view to a rendered scene.
class Camera {
  public:
    Camera();

    // Get the matrix that transforms world to view coordinates.
    glm::mat4 transform() const;

    // Get the camera position in world coordinates.
    glm::vec3 position() const;

    // Rotate the camera by (pitch, yaw) in degrees.
    void rotate(float, float);

    // Move the camera forwards along the view vector.
    void move_forward(float);

    // Move the camera backwards along the view vector.
    void move_backward(float);

    // Move the camera forwards along the up vector.
    void move_up(float);

    // Move the camera backwards along the up vector.
    void move_down(float);

    // Move the camera forwards along the right vector.
    void move_right(float);

    // Move the camera backwards along the right vector.
    void move_left(float);

  private:
    glm::vec3 _position;
    glm::vec3 _view_dir;
    glm::vec3 _up_dir;
    glm::vec3 _right_dir;
    float _pitch;
    float _yaw;
};
