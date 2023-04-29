#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/CameraController.hpp"

namespace Nyxis
{
    class Camera
    {
    public:
		Camera(TransformComponent& transform) : cameraTransform(transform) {}

		void OnUpdate(float dt);
        CameraType getType() { return cameraController.getCameraType(); }
        void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
        void setPerspectiveProjection(float fovy, float aspect, float near, float far);
        void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
        void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
        void setViewYXZ(glm::vec3 position, glm::vec3 rotation);
		void setMousePosition(glm::vec2 position) { cameraController.lastMousePosition = position; }

        [[nodiscard]] const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
        [[nodiscard]] const glm::mat4& getViewMatrix() const { return viewMatrix; }
        [[nodiscard]] const glm::mat4& getInverseViewMatrix() const { return inverseViewMatrix; }

        CameraController& getCameraController() { return cameraController; }
    private:
		TransformComponent& cameraTransform;
		CameraController cameraController;
        glm::mat4 projectionMatrix{1.0f};
        glm::mat4 viewMatrix{1.0f};
        glm::mat4 inverseViewMatrix{1.0f};
    };
}