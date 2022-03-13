#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = cameraTarget - cameraPosition;
        this->cameraRightDirection = glm::cross(this->cameraFrontDirection, this->cameraUpDirection);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(this->cameraPosition, this->cameraTarget, this->cameraUpDirection);
    }

    glm::vec3 Camera::getCameraDirection() {
        return this->cameraFrontDirection;
    }

    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //delta = modul din x2-x1/z2-z1
        glm::mat4 trans(1.0);
        if (direction == MOVE_FORWARD) {
            trans = glm::translate(trans, speed * glm::normalize(this->cameraFrontDirection));
        }
        if (direction == MOVE_BACKWARD) {
            trans = glm::translate(trans, -1 * speed * glm::normalize(this->cameraFrontDirection));
        }
        if (direction == MOVE_RIGHT) {
            trans = glm::translate(trans, speed * glm::normalize(this->cameraRightDirection));
        }
        if (direction == MOVE_LEFT) {
            trans = glm::translate(trans, -1 * speed * glm::normalize(this->cameraRightDirection));
        }

        this->cameraPosition = trans * glm::vec4(this->cameraPosition, 1.0);
        this->cameraTarget = trans * glm::vec4(this->cameraTarget, 1.0);

        this->cameraFrontDirection = cameraTarget - cameraPosition;
        this->cameraRightDirection = glm::cross(this->cameraFrontDirection, this->cameraUpDirection);
    }

    

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::mat4 trans(1.0);
        trans = glm::translate(trans, this->cameraPosition);


        trans = glm::rotate(trans, yaw, this->cameraUpDirection);
        trans = glm::rotate(trans, pitch, this->cameraRightDirection);
        trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, 0.0f) - this->cameraPosition);


        this->cameraTarget = trans * glm::vec4(this->cameraTarget, 1.0);
        this->cameraFrontDirection = cameraTarget - cameraPosition;
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));

    }
}