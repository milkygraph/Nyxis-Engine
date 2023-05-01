#include "Core/Camera.hpp"
#include "Core/Nyxispch.hpp"

namespace Nyxis
{
	void Camera::OnUpdate(float dt)
	{
		cameraController.processMouseMovement(dt, cameraTransform);
		setViewYXZ(cameraTransform.translation, cameraTransform.rotation);
	}

	void Camera::setOrthographicProjection(
		float left, float right, float top, float bottom, float near, float far)
	{
		projectionMatrix = glm::mat4{ 1.0f };
		projectionMatrix[0][0] = 2.f / (right - left);
		projectionMatrix[1][1] = 2.f / (bottom - top);
		projectionMatrix[2][2] = 1.f / (far - near);
		projectionMatrix[3][0] = -(right + left) / (right - left);
		projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		projectionMatrix[3][2] = -near / (far - near);
	}

	void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
	{
		projectionMatrix = glm::perspective(fovy, aspect, near, far);
	}

	void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation)
	{
		auto rotMatrix = glm::mat4{ 1.f };

		rotMatrix = glm::rotate(rotMatrix, rotation.x, glm::vec3{ 1.f, 0.f, 0.f });
		rotMatrix = glm::rotate(rotMatrix, rotation.y, glm::vec3{ 0.f, 1.f, 0.f });
		rotMatrix = glm::rotate(rotMatrix, rotation.z, glm::vec3{ 0.f, 0.f, 1.f });

		const auto transformMatrix = glm::translate(glm::mat4(1.0f), position * glm::vec3(1.0f, 1.0f, -1.0f));
		viewMatrix = rotMatrix * transformMatrix;
	}

}  // namespace Nyxis
