#pragma once

#include <BadgerEngine/Camera.h>
#include <BadgerEngine/Windows/SDL2Window.h>
#include <glm/glm.hpp>

// using Win = GLFWWindow;
using Win = BadgerEngine::SDL2Window;

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera : public BadgerEngine::Camera {

public:
    // Constructor with sensible defaults for common use cases
    // Provides flexibility while ensuring the camera starts in a predictable state
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), // Start at world origin
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), // Y-axis as world up
        float yaw = -90.0f, // Look along negative Z-axis (OpenGL convention)
        float pitch = 0.0f // Level horizon
        )
        : m_position(position)
        , m_up(up)
        , m_worldUp(up)
        , m_yaw(yaw)
        , m_pitch(pitch)
    {
    }

    // Matrix generation for graphics pipeline integration
    // These methods bridge between the camera's spatial representation and GPU requirements
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const;

    // Input processing methods for different interaction modalities
    // Each method handles a specific type of user input with appropriate transformations
    void processKeyboard(CameraMovement direction, float deltaTime); // Keyboard-based translation
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true); // Mouse-based rotation
    void processMouseScroll(float yOffset); // Scroll-based zoom control

    // Property access methods for external systems
    // Provide controlled access to internal state without exposing implementation details
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getFront() const { return m_front; }
    float getZoom() const { return m_zoom; }

    // Camera interface
public:
    glm::mat4 transformation(float aspect) const override;
    glm::vec3 position() const override;
    float near() const override;
    float far() const override;

private:
    // Internal coordinate system maintenance
    // Ensures mathematical consistency when orientation changes occur
    void updateCameraVectors();

private:
    // Spatial positioning and orientation vectors
    // These form the camera's local coordinate system in world space
    glm::vec3 m_position; // Camera's location in world coordinates
    glm::vec3 m_front; // Forward direction (where camera is looking)
    glm::vec3 m_up; // Camera's local up direction (for roll control)
    glm::vec3 m_right; // Camera's local right direction (perpendicular to front and up)
    glm::vec3 m_worldUp; // Global up vector reference (typically Y-axis)
    // Rotation representation using Euler angles
    // Provides intuitive control while managing gimbal lock and other rotation complexities
    float m_yaw; // Horizontal rotation around the world up-axis (left-right looking)
    float m_pitch; // Vertical rotation around the camera's right axis (up-down looking)

    // User interaction and behavior parameters
    // These control how the camera responds to input and environmental factors
    constexpr static float m_movementSpeed = 4; // Units per second for translation movement
    constexpr static float m_mouseSensitivity = 0.1; // Multiplier for mouse input to rotation angle conversion
    constexpr static float m_zoom = 45; // Field of view control for perspective projection
};

// Keyboard input processing for camera translation
// Handles discrete directional commands with frame-rate independent timing
void processInput(Camera& camera, const Win& window, float deltaTime);

// Mouse movement callback for continuous camera rotation
// Manages state persistence and coordinate transformations for smooth rotation control
void mouseCallback(Camera& camera, double xpos, double ypos);

void scrollCallback(Camera& camera, double xoffset, double yoffset);
