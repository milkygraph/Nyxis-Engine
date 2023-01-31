#include "CameraController.hpp"
#include "input.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
		}
		else
		{
			Input::setCursorMode(CursorMode::CursorNormal);
			lastMousePosition = Input::getMousePosition();
		}
	}
    void CameraController::moveInPlaneXZ(float dt, veGameObject &gameObject)
    {
		glm::vec3 rotate{0};
	    processMouseMovement(dt, gameObject);

	    float yaw = gameObject.transform.rotation.y;
	    const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
	    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
	    const glm::vec3 upDir{0.f, -.5f, 0.f};

	    if(Input::isKeyPressed(KeyCodes::LeftShift))
		    moveSpeed = 5.f;
	    else
		    moveSpeed = 3.f;

	    glm::vec3 moveDir{0.f};
	    if (Input::isKeyPressed(KeyCodes::W))
		    moveDir += forwardDir;
	    if (Input::isKeyPressed(KeyCodes::S))
		    moveDir -= forwardDir;
	    if (Input::isKeyPressed(KeyCodes::D))
		    moveDir += rightDir;
	    if (Input::isKeyPressed(KeyCodes::A))
		    moveDir -= rightDir;
	    if (Input::isKeyPressed(KeyCodes::E))
		    moveDir += upDir;
	    if (Input::isKeyPressed(KeyCodes::Q))
		    moveDir -= upDir;

	    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
	    {
		    gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
	    }
    }
}