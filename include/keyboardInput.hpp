#pragma once


#include "gameObject.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <limits>

namespace ve
{
    class CameraController
    {
    public:
        struct KeyMappings
        {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
            int fast = GLFW_KEY_LEFT_SHIFT;
        };

        void moveInPlaneXZ(float dt, veGameObject &gameObject);

    private:
        KeyMappings keys{};
        float moveSpeed{3.f};
        float lookSpeed{1.5f};
    };
}