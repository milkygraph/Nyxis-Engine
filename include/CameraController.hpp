#pragma once

#include "gameObject.hpp"
#include "window.hpp"
#include "input.hpp"
#include "Events/mouseEvents.hpp"
#include "Events/keyEvents.hpp"
#include "components.hpp"

#include "vepch.hpp"

namespace ve
{
	enum class CameraType
	{
		Orthographic,
		Perspective
	};
    class CameraController
    {
    public:
	    void processMouseMovement(float dt, TransformComponent& transform);
        void moveInPlaneXZ(float dt, TransformComponent& transform);
		float moveSpeed{10.f};
    private:
		glm::vec3 ForwardDir = {0.f, 0.f, 1.f};
        glm::vec3 RightDir = {1.f, 0.f, 0.f};
        glm::vec3 UpDir = {0.f, 1.f, 0.f};

		CameraType cameraType = CameraType::Orthographic;
        glm::vec2 lastMousePosition{0.f};
        float lookSpeed{1.5f};
		float rotationSpeed = 0.002f;
	};

	struct Player
	{
		float moveSpeed{ 1.f };

		void OnUpdate(float dt, TransformComponent& transform)
		{
			// 2d movement
			glm::vec3 moveDir{ 0.f };
			if (Input::isKeyPressed(KeyCodes::W))
				moveDir += glm::vec3{ 0.f, -1.f, 0.f };
			if (Input::isKeyPressed(KeyCodes::S))
				moveDir -= glm::vec3{ 0.f, -1.f, 0.f };
			if (Input::isKeyPressed(KeyCodes::D))
				moveDir += glm::vec3{ 1.f, 0.f, 0.f };
			if (Input::isKeyPressed(KeyCodes::A))
				moveDir -= glm::vec3{ 1.f, 0.f, 0.f };

			if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
			{
				transform.translation += moveSpeed * dt * glm::normalize(moveDir);
			}
		}
	};
}