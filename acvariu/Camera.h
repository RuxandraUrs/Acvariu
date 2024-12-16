// Camera.h
#ifndef CAMERA_H
#define CAMERA_H

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <glfw3.h>

enum ECameraMovementType
{
    UNKNOWN,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
private:
    // Default camera values
    const float zNEAR = 0.1f;
    const float zFAR = 200.f;
    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float FOV = 45.0f;
    glm::vec3 startPosition;


    

public:
    Camera(int width, int height, const glm::vec3& position);
    void Set(int width, int height, const glm::vec3& position);
    void Reset(int width, int height);
    void Reshape(int windowWidth, int windowHeight);
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::vec3 GetPosition() const;
    void ProcessKeyboard(int direction, float deltaTime);
    void MouseControl(float xPos, float yPos);
    void ProcessMouseScroll(float yOffset);

private:
    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch);
    void UpdateCameraVectors();

protected:
    const float cameraSpeedFactor = 200.5f;
    const float mouseSensitivity = 0.1f;

    // Perspective properties
    float zNear;
    float zFar;
    float FoVy;
    int width;
    int height;
    bool isPerspective;

    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp;

    // Euler Angles
    float yaw;
    float pitch;

    bool bFirstMouseMove = true;
    float lastX = 0.f, lastY = 0.f;
};

#endif