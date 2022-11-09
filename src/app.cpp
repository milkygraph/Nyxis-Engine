#include "app.hpp"
#include "model.hpp"
#include "renderer.hpp"
#include "simpleRenderSystem.hpp"
#include "swap_chain.hpp"

#include <iostream>
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
        while (!pWindow.shouldClose())
        {
            glfwPollEvents();
            if(auto commandBuffer = pRenderer.beginFrame())
            {
                pRenderer.beginSwapChainRenderPass(commandBuffer);
                srs.renderGameObjects(commandBuffer, gameObjects);
                pRenderer.endSwapChainRenderPass(commandBuffer);
                pRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(pDevice.device());
    }

    void App::loadGameObjects()
    {
        std::vector<veModel::Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

        auto pModel = std::make_shared<veModel>(pDevice, vertices);
        
        int num_of_triangles = 100;

        for (float i = 0; i < num_of_triangles; i += 1)
        {
            auto triangle = veGameObject::createGameObject();
            triangle.model = pModel;
            triangle.color = {.1f + i / num_of_triangles, .8f + i / num_of_triangles, .1f + i / num_of_triangles};
            triangle.transform2d.translation = {-0.2f + i / num_of_triangles / 3, -0.1f + i / num_of_triangles / 3};
            triangle.transform2d.scale = {0.6f, 0.6f};
            triangle.transform2d.rotation = .0f * glm::two_pi<float>();

            gameObjects.push_back(std::move(triangle));
        }
    }
} // namespace ve