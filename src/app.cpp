#include "app.hpp"
#include "components.hpp"
#include "descriptors.hpp"
#include "model.hpp"
#include "swap_chain.hpp"
#include "frameInfo.hpp"
#include "simpleRenderSystem.hpp"
#include "pointLightSystem.hpp"
#include "TextureRenderSystem.hpp"
#include "RenderSystems/ParticleRenderSystem.hpp"
#include "path.hpp"
#include "Log.hpp"
#include "GLTFRenderer.hpp"
#include "Nyxispch.hpp"
#include "imgui/misc/cpp/imgui_stdlib.h"

namespace Nyxis
{
    App* App::pInstance = nullptr;

    App::App()
	{
        Setup();
        loadGameObjects();
        pScene.LoadModels();
        pWindow.SetEventCallback(std::bind(&App::OnEvent, this, std::placeholders::_1));
        pInstance = this;
    }

    App::~App() = default;

    void App::OnEvent(Event& e)
	{
#ifdef LOGGING
#if LOGGING_LEVEL == 0
        std::string event_name = e.toString();
        LOG_INFO(event_name);
#endif // LOGGING_LEVEL
#endif // LOGGING
    }

    void App::Setup()
    {
        auto commandBuffer = pDevice.beginSingleTimeCommands();
        pImguiLayer.init(pRenderer.getSwapChainRenderPass(), commandBuffer);
        pDevice.endSingleTimeCommands(commandBuffer);
    }

    void App::run() {

        // create systems
        GLTFRenderer gltfRenderer{ pRenderer.getSwapChainRenderPass(), pRenderer.getSwapChainExtent() }; // gltfRenderer - gltfRenderer

        // add functions to imgui layer
        {
            pImguiLayer.AddFunction([&]() {
                ImGui::Begin("Statistics");
                ImGui::Text("Entity Count: %d", pScene.getEntityCount());
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                ImGui::End();
            });

            pImguiLayer.AddFunction([&]() {
                ImGui::Begin("Physics");
                ImGui::Checkbox("Enable Physics", &PhysicsEnabled);
                ImGui::DragFloat2("BoxEdges", &physicsEngine.edges.x);
                ImGui::DragFloat("Gravity", &physicsEngine.gravity, 0.1, -1.0f, 1.0f);
                ImGui::End();
            });
            pImguiLayer.AddFunction([&] {
                ImGui::Begin("Reload");
				
            	if (ImGui::Button("Reload", ImVec2(100, 50)) || ImGui::InputText("Path", &gltfRenderer.envMapFile, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    gltfRenderer.SceneUpdated = true;
                }

            	ImGui::End();
            });

			pImguiLayer.AddFunction([&]() {
				ImGui::Begin("Physics");
				ImGui::Checkbox("Enable Animations", &gltfRenderer.animate);
				ImGui::End();
				});
        }

    	auto currentTime = std::chrono::high_resolution_clock::now();

        veGameObject::Map map;

        std::thread animationThread;
        bool animationThreadActive = true;

        while (!pWindow.shouldClose()) {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            auto commandBuffer = pRenderer.beginFrame();

            float aspect = pRenderer.getAspectRatio();
            int frameIndex = pRenderer.getFrameIndex();
            FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, VK_NULL_HANDLE, map, pScene };

            pRenderer.beginSwapChainRenderPass(frameInfo.commandBuffer);

            gltfRenderer.Render(frameInfo);
            pImguiLayer.OnUpdate(frameInfo);
            if (PhysicsEnabled)
                physicsEngine.OnUpdate(pScene, frameInfo.frameTime);

            pRenderer.endSwapChainRenderPass(frameInfo.commandBuffer);
            pRenderer.endFrame();

            pScene.OnUpdate(frameInfo.frameTime, aspect);   
            gltfRenderer.OnUpdate();

            if (animationThreadActive) {
                animationThread = std::thread([&gltfRenderer, &frameInfo]() {
                    gltfRenderer.UpdateAnimation(frameInfo.frameTime);
                    });
                animationThread.detach();
            }
        }

        animationThreadActive = false;
        if (animationThread.joinable()) {
            animationThread.join();
        }

    	vkDeviceWaitIdle(pDevice.device());
    }

    void App::loadGameObjects()
    {
    }
} // namespace Nyxis