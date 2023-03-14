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
        pImguiLayer.init(pRenderer.GetUIRenderPass(), commandBuffer);
        pDevice.endSingleTimeCommands(commandBuffer);
    }

    void App::run() {

        std::thread animationThread;
        bool animationThreadActive = true;

        // create systems
        GLTFRenderer gltfRenderer{ pRenderer.GetSwapChainRenderPass() }; // gltfRenderer - gltfRenderer

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
            });

			pImguiLayer.AddFunction([&]() {
				ImGui::Begin("Physics");
				ImGui::Checkbox("Enable Animations", &animationThreadActive);
				ImGui::End();
				});

            pImguiLayer.AddFunction([&]()
            {
                ImGui::Begin("Scene Settings");
            	ImGui::Text("SkyMap");
				ImGui::DragFloat("Exposure", &gltfRenderer.shaderValuesParams.exposure, 0.1, 0.0f, 10.0f);
				ImGui::DragFloat("Gamma", &gltfRenderer.shaderValuesParams.gamma, 0.1, 0.0f, 10.0f);
				ImGui::DragFloat3("Light Direction", &gltfRenderer.shaderValuesParams.lightDir.x);

                if (ImGui::InputText("Path", &gltfRenderer.envMapFile, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Reload", ImVec2(75, 25)))
                {
                    gltfRenderer.SceneUpdated = true;
                }
            	ImGui::End();
            });
        }

        auto currentTime = std::chrono::high_resolution_clock::now();

        veGameObject::Map map;

        
        while (!pWindow.shouldClose()) {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

        	auto worldExtent = pRenderer.GetAspectRatio();
			float aspect = static_cast<float>(worldExtent.width) / static_cast<float>(worldExtent.height);
        	auto worldCommandBuffer = pRenderer.BeginWorldFrame();
        	int frameIndex = pRenderer.GetFrameIndex();

            FrameInfo frameInfo
            { frameIndex, frameTime, VK_NULL_HANDLE, VK_NULL_HANDLE, gameObjects, pScene };

			frameInfo.commandBuffer = worldCommandBuffer;

            pRenderer.BeginMainRenderPass(frameInfo.commandBuffer);
            gltfRenderer.Render(frameInfo);

            if (PhysicsEnabled)
                physicsEngine.OnUpdate(pScene, frameInfo.frameTime);
            pRenderer.EndMainRenderPass(worldCommandBuffer);
            
        	auto commandBuffer = pRenderer.BeginUIFrame();
			frameInfo.commandBuffer = commandBuffer;
        	auto extent = pImguiLayer.OnUpdate(frameInfo, pRenderer.GetWorldImageView(frameInfo.frameIndex));
            pRenderer.EndUIRenderPass(commandBuffer);
            pRenderer.m_WorldImageSize = extent;

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