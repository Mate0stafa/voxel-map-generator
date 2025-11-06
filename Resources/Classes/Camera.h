#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN
};

class Frustum {
public:
    void update(const glm::mat4& viewProjMatrix);
    bool isBoxVisible(const glm::vec3& min, const glm::vec3& max) const;
    
private:
    glm::vec4 planes[6];
};

class Camera {
public:
    // Camera attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    
    // Euler angles
    float Yaw;
    float Pitch;
    
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    
    // Frustum for culling
    Frustum frustum;
    
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f));
    
    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void UpdateFrustum(const glm::mat4& viewProjMatrix);
    
private:
    void updateCameraVectors();
};