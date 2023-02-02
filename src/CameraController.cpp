#include "CameraController.hpp"
#include "input.hpp"

namespace ve
{
	void CameraController::processMouseMovement(float dt, veGameObject &gameObject)
	{
		if(Input::isMouseButtonPressed(MouseCodes::MouseButtonRight))
		{
			Input::setCursorMode(CursorMode::CursorDisabled);
			glm::vec2 mousePosition = Input::getMousePosition();
			glm::vec2 mouseDelta = (mousePosition - lastMousePosition) * rotationSpeed;
			lastMousePosition = mousePosition;

			gameObject.transform.rotation.y += mouseDelta.x;
			gameObject.transform.rotation.x += -mouseDelta.y;

			gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());
			gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

            float yaw = gameObject.transform.rotation.y;
            ForwardDir = {sin(yaw), 0.f, cos(yaw)};
            RightDir = {ForwardDir.z, 0.f, -ForwardDir.x};
            UpDir = {0.f, -1.f, 0.f};

            moveInPlaneXZ(dt, gameObject);
        }
		else
		{
			Input::setCursorMode(CursorMode::CursorNormal);
			lastMousePosition = Input::getMousePosition();
		}
	}
    void CameraController::moveInPlaneXZ(float dt, veGameObject &gameObject)
    {

	    glm::vec3 moveDir{0.f};
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

	    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
	    {
		    gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
	    }
    }
}