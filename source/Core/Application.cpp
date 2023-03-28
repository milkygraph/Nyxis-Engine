#include "Core/Application.hpp"
#include "Core/Renderer.hpp"
#include "Core/GLTFRenderer.hpp"
#include "Core/FrameInfo.hpp"
#include "Events/MouseEvents.hpp"
#include "Scene/Components.hpp"

namespace Nyxis
{
    Application::Application()
	{
        Renderer::Init(&m_Window, &m_Device);
        m_FrameInfo = std::make_shared<FrameInfo>();
        m_EditorLayer.OnAttach();
    	auto commandBuffer = m_Device.beginSingleTimeCommands();
        m_EditorLayer.Init(Renderer::GetUIRenderPass(), commandBuffer);
        m_Device.endSingleTimeCommands(commandBuffer);
    	m_Scene = std::make_shared<Scene>();
        m_EditorLayer.SetScene(m_Scene);
    	s_Instance = this;
        m_Window.SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
	}

    Application::~Application()
    {
        m_EditorLayer.OnDetach();
        Renderer::Shutdown();
    }

    void Application::OnEvent(Event& e)
	{
#if 1
		LOG_INFO(e.toString());
#endif
	}

    void Application::Run()
	{

        std::thread animationThread;
        bool animationThreadActive = true;

        // create systems
        GLTFRenderer gltfRenderer{ Renderer::GetSwapChainRenderPass() };

        // add functions to editor layer
        {
            m_EditorLayer.AddFunction([&]() {
                ImGui::Begin("Statistics");
                ImGui::Text("Entity Count: %d", m_Scene->getEntityCount());
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                ImGui::End();
            });

            m_EditorLayer.AddFunction([&]() {
                ImGui::Begin("Physics");
                ImGui::Checkbox("Enable Physics", &PhysicsEnabled);
                ImGui::DragFloat2("BoxEdges", &m_PhysicsEngine.edges.x);
                ImGui::DragFloat("Gravity", &m_PhysicsEngine.gravity, 0.1, -1.0f, 1.0f);
                ImGui::End();
            });
            m_EditorLayer.AddFunction([&] {
            });

			m_EditorLayer.AddFunction([&]() {
				ImGui::Begin("Physics");
				ImGui::Checkbox("Enable Animations", &animationThreadActive);
				ImGui::End();
			});

            m_EditorLayer.AddFunction([&]()
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

        auto model2 = m_Scene->createEntity("Microphone");
        m_Scene->addComponent<Model>(model2, "../models/microphone/scene.gltf", gltfRenderer.sceneInfo, gltfRenderer.uniformBuffersParams);
        m_Scene->addComponent<RigidBody>(model2);

    	while (!m_Window.ShouldClose()) {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            auto frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

        	auto worldExtent = Renderer::GetAspectRatio();
			float aspect = static_cast<float>(worldExtent.width) / static_cast<float>(worldExtent.height);
        	auto worldCommandBuffer = Renderer::BeginWorldFrame();

			m_FrameInfo->frameTime = frameTime;
			m_FrameInfo->frameIndex = Renderer::GetFrameIndex();
    		m_FrameInfo->commandBuffer = worldCommandBuffer;

            Renderer::BeginMainRenderPass(m_FrameInfo->commandBuffer);
            gltfRenderer.Render();

            if (PhysicsEnabled)
                m_PhysicsEngine.OnUpdate(m_FrameInfo->frameTime);
            Renderer::EndMainRenderPass(worldCommandBuffer);
            
        	auto commandBuffer = Renderer::BeginUIFrame();
			m_FrameInfo->commandBuffer = commandBuffer;

            m_EditorLayer.Begin();
    		m_EditorLayer.OnUpdate();
            m_EditorLayer.End();
    		Renderer::EndUIRenderPass(commandBuffer);
			Renderer::SetWorldImageSize(m_EditorLayer.GetViewportExtent());

            m_Scene->OnUpdate(m_FrameInfo->frameTime, aspect);

    		gltfRenderer.OnUpdate();
            if (animationThreadActive) {
                animationThread = std::thread([&]{
					gltfRenderer.UpdateAnimation(m_FrameInfo->frameTime);
                    });
                animationThread.detach();
            }
        }

        animationThreadActive = false;
        if (animationThread.joinable()) {
            animationThread.join();
        }

    	vkDeviceWaitIdle(m_Device.device());
    }
} // namespace Nyxis