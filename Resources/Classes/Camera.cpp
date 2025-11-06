//
// Created by mateo on 5/18/25.
//

#include "Camera.h"
#include <algorithm>

Camera::Camera(glm::vec3 position) : 
    Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(2.5f),
    MouseSensitivity(0.1f),
    Zoom(45.0f)
{
    Position = position;
    WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    Yaw = -90.0f;
    Pitch = 0.0f;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += WorldUp * velocity;
    if (direction == DOWN)
        Position -= WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        Pitch = std::clamp(Pitch, -89.0f, 89.0f);
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    Zoom = std::clamp(Zoom - yoffset, 1.0f, 45.0f);
}

void Camera::UpdateFrustum(const glm::mat4& viewProjMatrix) {
    frustum.update(viewProjMatrix);
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

// Frustum implementation
void Frustum::update(const glm::mat4& viewProjMatrix) {
    // Extract frustum planes from view-projection matrix
    // Left plane
    planes[0] = glm::vec4(
        viewProjMatrix[0][3] + viewProjMatrix[0][0],
        viewProjMatrix[1][3] + viewProjMatrix[1][0],
        viewProjMatrix[2][3] + viewProjMatrix[2][0],
        viewProjMatrix[3][3] + viewProjMatrix[3][0]
    );
    
    // Right plane
    planes[1] = glm::vec4(
        viewProjMatrix[0][3] - viewProjMatrix[0][0],
        viewProjMatrix[1][3] - viewProjMatrix[1][0],
        viewProjMatrix[2][3] - viewProjMatrix[2][0],
        viewProjMatrix[3][3] - viewProjMatrix[3][0]
    );
    
    // Bottom plane
    planes[2] = glm::vec4(
        viewProjMatrix[0][3] + viewProjMatrix[0][1],
        viewProjMatrix[1][3] + viewProjMatrix[1][1],
        viewProjMatrix[2][3] + viewProjMatrix[2][1],
        viewProjMatrix[3][3] + viewProjMatrix[3][1]
    );
    
    // Top plane
    planes[3] = glm::vec4(
        viewProjMatrix[0][3] - viewProjMatrix[0][1],
        viewProjMatrix[1][3] - viewProjMatrix[1][1],
        viewProjMatrix[2][3] - viewProjMatrix[2][1],
        viewProjMatrix[3][3] - viewProjMatrix[3][1]
    );
    
    // Near plane
    planes[4] = glm::vec4(
        viewProjMatrix[0][3] + viewProjMatrix[0][2],
        viewProjMatrix[1][3] + viewProjMatrix[1][2],
        viewProjMatrix[2][3] + viewProjMatrix[2][2],
        viewProjMatrix[3][3] + viewProjMatrix[3][2]
    );
    
    // Far plane
    planes[5] = glm::vec4(
        viewProjMatrix[0][3] - viewProjMatrix[0][2],
        viewProjMatrix[1][3] - viewProjMatrix[1][2],
        viewProjMatrix[2][3] - viewProjMatrix[2][2],
        viewProjMatrix[3][3] - viewProjMatrix[3][2]
    );
    
    // Normalize planes
    for (auto& plane : planes) {
        float length = glm::length(glm::vec3(plane));
        plane /= length;
    }
}

bool Frustum::isBoxVisible(const glm::vec3& min, const glm::vec3& max) const {
    for (const auto& plane : planes) {
        // Check if all 8 corners of the box are outside any plane
        int out = 0;
        out += (glm::dot(plane, glm::vec4(min.x, min.y, min.z, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(max.x, min.y, min.z, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(min.x, max.y, min.z, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(max.x, max.y, min.z, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(min.x, min.y, max.z, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(max.x, min.y, max.z, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(min.x, max.y, max.z, 1.0f)) < 0.0f) ? 1 : 0;
        out += (glm::dot(plane, glm::vec4(max.x, max.y, max.z, 1.0f)) < 0.0f) ? 1 : 0;
        
        if (out == 8) return false;
    }
    return true;
}