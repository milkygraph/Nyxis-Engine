#include "CameraController.hpp"

namespace ve
{
	void CameraController::processMouseMovement(float dt, TransformComponent& transform)
	{
		if(Input::isMouseButtonPressed(MouseCodes::MouseButtonRight) && cameraType == CameraType::Perspective)
		{
			Input::setCursorMode(CursorMode::CursorDisabled);
			glm::vec2 mousePosition = Input::getMousePosition();
			glm::vec2 mouseDelta = (mousePosition - lastMousePosition) * rotationSpeed;
			lastMousePosition = mousePosition;

			transform.rotation.y += mouseDelta.x;
			transform.rotation.x += -mouseDelta.y;

			transform.rotation.x = glm::clamp(transform.rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());
			transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

            float yaw = transform.rotation.y;
            ForwardDir = {sin(yaw), 0.f, cos(yaw)};
            RightDir = {ForwardDir.z, 0.f, -ForwardDir.x};
            UpDir = {0.f, -1.f, 0.f};

            moveInPlaneXZ(dt, transform);
        }
		else
		{
			Input::setCursorMode(CursorMode::CursorNormal);
			lastMousePosition = Input::getMousePosition();
		}
	}
    void CameraController::moveInPlaneXZ(float dt, TransformComponent& transform)
    {
		glm::vec3 moveDir{ 0.f };
		if(cameraType == CameraType::Perspective)
		{
			if (Input::isKeyPressed(KeyCodes::W))
				moveDir += ForwardDir;
			if (Input::isKeyPressed(KeyCodes::S))
				moveDir -= ForwardDir;
			if (Input::isKeyPressed(KeyCodes::D))
				moveDir += RightDir;
			if (Input::isKeyPressed(KeyCodes::A))
				moveDir -= RightDir;
			if (Input::isKeyPressed(KeyCodes::E))
				moveDir += UpDir;
			if (Input::isKeyPressed(KeyCodes::Q))
				moveDir -= UpDir;
		}
		else
		{
			if (Input::isKeyPressed(KeyCodes::W))
				moveDir += glm::vec3{ 0.f, -1.f, 0.f };
			if (Input::isKeyPressed(KeyCodes::S))
				moveDir -= glm::vec3{ 0.f, -1.f, 0.f };
			if (Input::isKeyPressed(KeyCodes::D))
				moveDir += glm::vec3{ 1.f, 0.f, 0.f };
			if (Input::isKeyPressed(KeyCodes::A))
				moveDir -= glm::vec3{ 1.f, 0.f, 0.f };
		}

	    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
	    {
		    transform.translation += moveSpeed * dt * glm::normalize(moveDir);
	    }
    }
}