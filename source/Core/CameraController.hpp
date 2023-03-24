#pragma once
#include "Scene/Components.hpp"
#include "Core/Nyxispch.hpp"

namespace Nyxis
{
	enum class CameraType
	{
		Orthographic,
		Perspective
	};
    class CameraController
    {
    public:
	    void processMouseMovement(float dt, RigidBody& rigidBody);
        void moveInPlaneXZ(float dt, RigidBody& rigidBody);
		void setCameraType(CameraType type) { cameraType = type; }
		CameraType getCameraType() { return cameraType; }
		float moveSpeed{10.f};
    private:
		friend class Camera;
		glm::vec3 ForwardDir = {0.f, 0.f, 1.f};
        glm::vec3 RightDir = {1.f, 0.f, 0.f};
        glm::vec3 UpDir = {0.f, 1.f, 0.f};

		CameraType cameraType = CameraType::Orthographic;
        glm::vec2 lastMousePosition{0.f};
        float lookSpeed{1.5f};
		float rotationSpeed = 0.002f;
	};
}