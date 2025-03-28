﻿#include "Camera.h"

Camera::Camera(int width, int height, const glm::vec3& position)
{
    startPosition = position;
    Set(width, height, position);
}

void Camera::Set(int width, int height, const glm::vec3& position) {
    this->isPerspective = true;
    this->yaw = YAW;
    this->pitch = PITCH;

    this->FoVy = FOV;
    this->width = width;
    this->height = height;
    this->zNear = zNEAR;
    this->zFar = zFAR;

    this->worldUp = glm::vec3(0, 1, 0);
    this->position = position;

    lastX = width / 2.0f;
    lastY = height / 2.0f;
    bFirstMouseMove = true;

    UpdateCameraVectors();
}

void Camera::Reset(int width, int height) {
    Set(width, height, position);
}

void Camera::Reshape(int windowWidth, int windowHeight) {
    width = windowWidth;
    height = windowHeight;
    glViewport(0, 0, windowWidth, windowHeight);
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position, position + forward, up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    glm::mat4 Proj = glm::mat4(1);
    if (isPerspective) {
        float aspectRatio = ((float)(width)) / height;
        Proj = glm::perspective(glm::radians(FoVy), aspectRatio, zNear, zFar);
    }
    else {
        float scaleFactor = 2000.f;
        Proj = glm::ortho<float>(
            -width / scaleFactor, width / scaleFactor,
            -height / scaleFactor, height / scaleFactor, -zFar, zFar);
    }
    return Proj;
}

glm::vec3 Camera::GetPosition() const {
    return position;
}

void Camera::ProcessKeyboard(int direction, float deltaTime) {
    float velocity = (float)(cameraSpeedFactor * deltaTime);
    switch (direction) {
    case ECameraMovementType::FORWARD:
        position += forward * velocity;
        break;
    case ECameraMovementType::BACKWARD:
        position -= forward * velocity;
        break;
    case ECameraMovementType::LEFT:
        position -= right * velocity;
        break;
    case ECameraMovementType::RIGHT:
        position += right * velocity;
        break;
    case ECameraMovementType::UP:
        position += up * velocity;
        break;
    case ECameraMovementType::DOWN:
        position -= up * velocity;
        break;
    }
}

void Camera::MouseControl(float xPos, float yPos) {
    if (bFirstMouseMove) {
        lastX = xPos;
        lastY = yPos;
        bFirstMouseMove = false;
    }

    float xChange = xPos - lastX;
    float yChange = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    if (fabs(xChange) <= 1e-6 && fabs(yChange) <= 1e-6) {
        return;
    }
    xChange *= mouseSensitivity;
    yChange *= mouseSensitivity;

    ProcessMouseMovement(xChange, yChange, true);
}

void Camera::ProcessMouseScroll(float yOffset) {
    if (FoVy >= 1.0f && FoVy <= 90.0f) {
        FoVy -= yOffset;
    }
    if (FoVy <= 1.0f)
        FoVy = 1.0f;
    if (FoVy >= 90.0f)
        FoVy = 90.0f;
}
void Camera::RotateAroundPoint(const glm::vec3& point, float angle, float distance) {
 
    float newX = point.x + distance * cos(glm::radians(angle));
    float newZ = point.z + distance * sin(glm::radians(angle));


    pitch = 0.0f;
    position = glm::vec3(newX, pitch, newZ); 

   
    

  
    forward = glm::normalize(point - position);
    right = glm::normalize(glm::cross(forward, worldUp));
    up = glm::normalize(glm::cross(right, forward));
}


void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
    constrainPitch = true;
    yaw += xOffset;
    pitch += yOffset;


    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

   
    UpdateCameraVectors();
}
void Camera::UpdateCameraVectors() {
   
    this->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->forward.y = sin(glm::radians(pitch));
    this->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->forward = glm::normalize(this->forward);

    right = glm::normalize(glm::cross(forward, worldUp));
    up = glm::normalize(glm::cross(right, forward));
}