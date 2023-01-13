#include "keyboardInput.hpp"
#include <GLFW/glfw3.h>
#include "input.hpp"

namespace ve
{
    void CameraController::moveInPlaneXZ(GLFWwindow *window, float dt, veGameObject &gameObject)
    {
        glm::vec3 rotate{0};
        if (Input::isKeyPressed(window, keys.lookRight))
            rotate.y += .5f;
        if (Input::isKeyPressed(window, keys.lookLeft))
            rotate.y -= .5f;
        if (Input::isKeyPressed(window, keys.lookUp))
            rotate.x += .5f;
        if (Input::isKeyPressed(window, keys.lookDown))
            rotate.x -= .5f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
            gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);

        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, -.5f, 0.f};

        if(Input::isKeyPressed(window, keys.fast))
            moveSpeed = 5.f;
        else
            moveSpeed = 3.f;

        glm::vec3 moveDir{0.f};
        if (Input::isKeyPressed(window, keys.moveForward))
            moveDir += forwardDir;
        if (Input::isKeyPressed(window, keys.moveBackward))
            moveDir -= forwardDir;
        if (Input::isKeyPressed(window, keys.moveRight))
            moveDir += rightDir;
        if (Input::isKeyPressed(window, keys.moveLeft))
            moveDir -= rightDir;
        if (Input::isKeyPressed(window, keys.moveUp))
            moveDir += upDir;
        if (Input::isKeyPressed(window, keys.moveDown))
            moveDir -= upDir;

        static double xpos, ypos;
        // glfwGetCursorPos(window, xpos, ypos);

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}