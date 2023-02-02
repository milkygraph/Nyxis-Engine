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
        float moveSpeed{10.f};
    private:
		glm::vec3 ForwardDir = {0.f, 0.f, 1.f};
        glm::vec3 RightDir = {1.f, 0.f, 0.f};
        glm::vec3 UpDir = {0.f, 1.f, 0.f};

        glm::vec2 lastMousePosition{0.f};
        float lookSpeed{1.5f};
		float rotationSpeed = 0.002f;
    };
}