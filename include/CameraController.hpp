#pragma once

#include "gameObject.hpp"
#include "window.hpp"
#include "Events/mouseEvents.hpp"
#include "Events/keyEvents.hpp"

#include "vepch.hpp"

namespace ve
{
    class CameraController
    {
    public:
	    void processMouseMovement(float dt, veGameObject &gameObject);
        void moveInPlaneXZ(float dt, veGameObject &gameObject);

    private:
		glm::vec2 lastMousePosition{0.f};
        float moveSpeed{3.f};
        float lookSpeed{1.5f};
		float rotationSpeed = 0.002f;
    };
}