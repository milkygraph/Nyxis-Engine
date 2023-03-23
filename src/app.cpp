#include "app.hpp"
#include "components.hpp"
#include "model.hpp"
#include "frameInfo.hpp"
#include "simpleRenderSystem.hpp"
#include "Log.hpp"
#include "GLTFRenderer.hpp"
#include "Nyxispch.hpp"

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
#if LOGGING_LEVEL == 1
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
				ImGui::DragFloat("Exposure", &gltfRenderer.sceneInfo.shaderValuesParams.exposure, 0.1, 0.0f, 10.0f);
				ImGui::DragFloat("Gamma", &gltfRenderer.sceneInfo.shaderValuesParams.gamma, 0.1, 0.0f, 10.0f);
				ImGui::DragFloat3("Light Direction", &gltfRenderer.sceneInfo.shaderValuesParams.lightDir.x);
                if (ImGui::InputText("Path", &gltfRenderer.envMapFile, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Reload", ImVec2(75, 25)))
                {
                    gltfRenderer.SceneUpdated = true;
                }

				static std::vector<const char *> debugViews = { "None", "Base color", "Normal", "Occlusion", "Emissive", "Metallic", "Roughness" };
            	static int debugViewIndex = 0;
                if (ImGui::Combo("Debug View", &debugViewIndex, &debugViews[0], debugViews.size(), debugViews.size()))
                    gltfRenderer.sceneInfo.shaderValuesParams.debugViewInputs = static_cast<float>(debugViewIndex);

                static std::vector<const char*> pbrEquations = { "None", "Diff (l,n)", "F (l,h)", "G (l,v,h)", "D (h)", "Specular" };
                static int pbrIndex = 0;
                if (ImGui::Combo("PBR Equation", &pbrIndex, &pbrEquations[0], pbrEquations.size(), pbrEquations.size()))
                    gltfRenderer.sceneInfo.shaderValuesParams.debugViewEquation = static_cast<float>(pbrIndex);

            	ImGui::End();
            });
        }

        auto currentTime = std::chrono::high_resolution_clock::now();

        veGameObject::Map map;

        auto model2 = pScene.createEntity("Microphone");
        pScene.addComponent<Model>(model2, "../models/microphone/scene.gltf", gltfRenderer.sceneInfo, gltfRenderer.uniformBuffersParams);
        pScene.addComponent<RigidBody>(model2);

		auto model3 = pScene.createEntity("Cute Robot");
        pScene.addComponent<Model>(model3, "../models/roboto/scene.gltf", gltfRenderer.sceneInfo, gltfRenderer.uniformBuffersParams);
        pScene.addComponent<RigidBody>(model3);

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

            pImguiLayer.Begin();
            auto extent = pImguiLayer.OnUpdate(frameInfo, pRenderer.GetWorldImageView(frameInfo.frameIndex));
            pImguiLayer.End();
    		pRenderer.EndUIRenderPass(commandBuffer);
            pRenderer.m_WorldImageSize = extent;

            pScene.OnUpdate(frameInfo.frameTime, aspect);

    		gltfRenderer.OnUpdate(frameInfo.scene);
            if (animationThreadActive) {
                animationThread = std::thread([&gltfRenderer, &frameInfo]() {
					gltfRenderer.UpdateAnimation(frameInfo);
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