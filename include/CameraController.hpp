#pragma once

#include "gameObject.hpp"
#include "window.hpp"
#include "Events/mouseEvents.hpp"
#include "Events/keyEvents.hpp"

#include <GLFW/glfw3.h>

#include <limits>

namespace ve
{
    class CameraController
    {
    public:
        void moveInPlaneXZ(float dt, veGameObject &gameObject);

    private:
	    void processMouseMovement(float dt, veGameObject &gameObject);
		glm::quat orientation = glm::quat(1, 0, 0, 0);
		glm::vec2 lastMousePosition{0.f};
        float moveSpeed{3.f};
        float lookSpeed{1.5f};
		float rotationSpeed = 0.002f;
    };
}