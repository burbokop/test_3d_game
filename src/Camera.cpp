#include "Camera.h"

#include <algorithm>
#include <glm/ext.hpp>

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio, float nearPlane, float farPlane) const
{
    return glm::perspective(glm::radians(m_zoom), aspectRatio, nearPlane, farPlane);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = m_movementSpeed * deltaTime;

    switch (direction) {
    case CameraMovement::FORWARD:
        m_position += m_front * velocity;
        break;
    case CameraMovement::BACKWARD:
        m_position -= m_front * velocity;
        break;
    case CameraMovement::LEFT:
        m_position -= m_right * velocity;
        break;
    case CameraMovement::RIGHT:
        m_position += m_right * velocity;
        break;
    case CameraMovement::UP:
        m_position -= m_up * velocity;
        break;
    case CameraMovement::DOWN:
        m_position += m_up * velocity;
        break;
    }
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
    xOffset *= m_mouseSensitivity;
    yOffset *= m_mouseSensitivity;

    m_yaw += xOffset;
    m_pitch += yOffset;

    // Constrain pitch to avoid flipping
    if (constrainPitch) {
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
    }

    // Update camera vectors based on updated Euler angles
    updateCameraVectors();
}

void Camera::processMouseScroll(float yOffset)
{
}

void Camera::updateCameraVectors()
{
    // Calculate the new front vector
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    newFront.y = sin(glm::radians(m_pitch));
    newFront.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(newFront);

    // Recalculate the right and up vectors
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

glm::mat4 Camera::transformation(glm::vec2 extent) const
{
    return getProjectionMatrix(extent.x / extent.y, near(), far()) * getViewMatrix();
}

glm::vec3 Camera::position() const
{
    return m_position;
}

float Camera::near() const
{
    return 0.1;
}

float Camera::far() const
{
    return 100;
}

void processInput(Camera& camera, const Win& window, float deltaTime)
{
    // WASD movement scheme following standard FPS conventions
    // Each key press translates to a specific directional movement relative to camera orientation
    if (window.isPressed(Win::Key::W))
        camera.processKeyboard(CameraMovement::FORWARD, deltaTime); // Move forward along camera's front vector
    if (window.isPressed(Win::Key::S))
        camera.processKeyboard(CameraMovement::BACKWARD, deltaTime); // Move backward opposite to front vector
    if (window.isPressed(Win::Key::A))
        camera.processKeyboard(CameraMovement::LEFT, deltaTime); // Strafe left along camera's right vector
    if (window.isPressed(Win::Key::D))
        camera.processKeyboard(CameraMovement::RIGHT, deltaTime); // Strafe right along camera's right vector

    // Vertical movement controls for 3D navigation
    // Space and Control provide intuitive up/down movement
    if (window.isPressed(Win::Key::SPACE))
        camera.processKeyboard(CameraMovement::UP, deltaTime); // Move up along camera's up vector
    if (window.isPressed(Win::Key::LEFT_CONTROL))
        camera.processKeyboard(CameraMovement::DOWN, deltaTime); // Move down opposite to up vector
}

void mouseCallback(Camera& camera, double xpos, double ypos)
{
    // State persistence for calculating movement deltas
    // Static variables maintain state between callback invocations
    static bool firstMouse = true; // Flag to handle initial mouse position
    static float lastX = 0.0f, lastY = 0.0f; // Previous mouse position for delta calculation

    // Handle initial mouse position to prevent sudden camera jumps
    // First callback provides absolute position, not relative movement
    if (firstMouse) {
        lastX = xpos; // Initialize previous position
        lastY = ypos;
        firstMouse = false; // Disable special handling for subsequent calls
    }

    // Calculate mouse movement deltas since last callback
    // These deltas represent the amount and direction of mouse movement
    float xoffset = xpos - lastX; // Horizontal movement (left-right)
    float yoffset = ypos - lastY; // Vertical movement (inverted: screen Y increases downward, camera pitch increases upward)

    // Update state for next callback iteration
    lastX = xpos;
    lastY = ypos;

    // Convert mouse movement to camera rotation
    // Delta values drive continuous camera orientation changes
    camera.processMouseMovement(xoffset, yoffset);
}

void scrollCallback(Camera& camera, double xoffset, double yoffset)
{
    // Direct scroll-to-zoom mapping
    // Positive yoffset (scroll up) typically zooms in, negative (scroll down) zooms out
    camera.processMouseScroll(yoffset);
}
