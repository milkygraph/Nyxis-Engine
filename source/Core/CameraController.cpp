#include "CameraController.hpp"

namespace Nyxis
{
	void CameraController::processMouseMovement(float dt, TransformComponent& transform)
	{
		glm::vec2 mousePosition = Input::GetMousePosition();
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
    void CameraController::moveInPlaneXZ(float dt, TransformComponent& transform)
    {
		glm::vec3 moveDir{ 0.f };
		if(cameraType == CameraType::Perspective)
		{
			if (Input::IsKeyPressed(KeyCodes::W))
				moveDir -= ForwardDir;
			if (Input::IsKeyPressed(KeyCodes::S))
				moveDir += ForwardDir;
			if (Input::IsKeyPressed(KeyCodes::D))
				moveDir -= RightDir;
			if (Input::IsKeyPressed(KeyCodes::A))
				moveDir += RightDir;
			if (Input::IsKeyPressed(KeyCodes::E))
				moveDir -= UpDir;
			if (Input::IsKeyPressed(KeyCodes::Q))
				moveDir += UpDir;
		}
		else
		{
			if (Input::IsKeyPressed(KeyCodes::W))
				moveDir += glm::vec3{ 0.f,1.f, 0.f };
			if (Input::IsKeyPressed(KeyCodes::S))
				moveDir -= glm::vec3{ 0.f, -1.f, 0.f };
			if (Input::IsKeyPressed(KeyCodes::D))
				moveDir += glm::vec3{ 1.f, 0.f, 0.f };
			if (Input::IsKeyPressed(KeyCodes::A))
				moveDir -= glm::vec3{ 1.f, 0.f, 0.f };
		}

	    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
	    {
		    transform.translation += moveSpeed * dt * glm::normalize(moveDir);
	    }
    }
}