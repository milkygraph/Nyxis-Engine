#include "app.hpp"
#include "model.hpp"
#include "renderer.hpp"
#include "simpleRenderSystem.hpp"
#include "swap_chain.hpp"
#include "camera.hpp"
#include "keyboardInput.hpp"

#include <iostream>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <array>

namespace ve
{
    App::App()
    {
        loadGameObjects();
    }

    App::~App()
    {
    }

    void App::run()
    {
        SimpleRenderSystem srs{pDevice, pRenderer.getSwapChainRenderPass()}; // srs - simpleRenderSystem
        veCamera camera{};

        auto viewerObject = veGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!pWindow.shouldClose())
        {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(pWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = pRenderer.getAspectRatio();
            // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.01f, 100.f);
            if (auto commandBuffer = pRenderer.beginFrame())
            {
                pRenderer.beginSwapChainRenderPass(commandBuffer);
                srs.renderGameObjects(commandBuffer, gameObjects, camera);
                pRenderer.endSwapChainRenderPass(commandBuffer);
                pRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(pDevice.device());
    }

    void App::loadGameObjects()
    {
        auto time1 = std::chrono::high_resolution_clock::now();

        std::shared_ptr<veModel> model = veModel::createModelFromFile(pDevice, "models/Srad 750.obj");

        auto oto1 = veGameObject::createGameObject();
        oto1.model = model;
        oto1.transform.translation = {.5f, .5f, .0f};
        oto1.transform.rotation = {.0f, .0f, 0.0f};
        oto1.transform.scale = {.5f, .5f, .5f};
        
        auto newTime = std::chrono::high_resolution_clock::now();
        auto duration1 = std::chrono::duration<float, std::chrono::seconds::period>(newTime - time1).count();
        std::cout << duration1 << std::endl;

        gameObjects.push_back(std::move(oto1));
    }

} // namespace ve