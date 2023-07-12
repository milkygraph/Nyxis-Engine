#include "Core/Application.hpp"

#include "Pipeline.hpp"
#include "Core/Renderer.hpp"
#include "Core/GLTFRenderer.hpp"
#include "Core/FrameInfo.hpp"
#include "Events/MouseEvents.hpp"
#include "Scene/Components.hpp"
#include "Scene/NyxisProject.hpp"

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
        m_CurrentProject = std::make_shared<NyxisProject>("Default", "default_project.npj");
        m_CurrentProject->Create();
        m_CurrentProject->AddScene(m_Scene);
        m_EditorLayer.SetScene(m_Scene);
    	s_Instance = this;
        m_Window.SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
	}

    Application::~Application()
    {
        m_EditorLayer.OnDetach();
		GLTFRenderer::Shutdown();
        Renderer::Shutdown();
    }

    void Application::OnEvent(Event& e)
	{
		m_EditorLayer.OnEvent(e);
		GLTFRenderer::OnEvent(e);
	}
    // TODO: Move to editor layer
    void AddPipelineConfigUI(const char* name, Ref<Pipeline> pipeline, PipelineType type)
    {
		if (!pipeline)
			return;
        auto cullingMode = pipeline->pipelineConfigInfo.rasterizationInfo.cullMode;
        auto frontFace = pipeline->pipelineConfigInfo.rasterizationInfo.frontFace;

        static std::vector cullingModes = { "None", "Front", "Back", "Front and Back" };
        static std::vector frontFaces = { "Counter Clockwise", "Clockwise" };
        static std::vector depthModes = { "None", "Read Only", "Write Only", "Read/Write" };
        static std::vector blendOperations = { "Add", "Subtract", "Min", "Max", "Zero", "Src", "Dst" };
        static std::vector stencilOperations = { "Keep", "Zero", "Replace", "Increment and Clamp", "Decrement and Clamp", "Invert", "Increment and Wrap", "Decrement and Wrap" };
        static std::vector stencilTestModes = { "Never", "Less", "Equal", "Less or Equal", "Greater", "Not Equal", "Greater or Equal", "Always" };
        static std::vector polygonModes = { "Fill", "Line", "Point" };
        static std::vector depthBiasModes = { "None", "Dynamic", "Static" };
        static std::vector blendFactor = { "Zero", "One", "Src Color", "One Minus Src Color", "Dst Color",
            "One Minus Dst Color", "Src Alpha", "One Minus Src Alpha", "Dst Alpha", "One Minus Dst Alpha", "Constant Color", "One Minus Constant Color",
            "Constant Alpha", "One Minus Constant Alpha", "Src Alpha Saturate" };

        ImGui::PushID(name);

        ImGui::Text(name);
        if(ImGui::TreeNode("Culling"))
        {
            ImGui::TreePop();
			if (ImGui::BeginCombo("Culling", cullingModes[static_cast<VkCullModeFlagBits>(cullingMode)]))
			{
				for (int i = 0; i < cullingModes.size(); ++i)
				{
					const bool is_selected = (cullingMode == i);
					if (ImGui::Selectable(cullingModes[i], is_selected))
					{
						cullingMode = i;
						pipeline->pipelineConfigInfo.rasterizationInfo.cullMode = static_cast<VkCullModeFlagBits>(i);
						pipeline->pipelineConfigInfo.renderPass = Renderer::GetSwapChainRenderPass();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Front Face", frontFaces[static_cast<int>(frontFace)]))
			{
				for (int i = 0; i < frontFaces.size(); ++i)
				{
					const bool is_selected = (frontFace == i);
					if (ImGui::Selectable(frontFaces[i], is_selected))
					{
						frontFace = static_cast<VkFrontFace>(i);
						pipeline->pipelineConfigInfo.rasterizationInfo.frontFace = frontFace;
						pipeline->pipelineConfigInfo.renderPass = Renderer::GetSwapChainRenderPass();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
        }

        if(ImGui::TreeNode("Depth"))
        {
			ImGui::TreePop();
			if (ImGui::BeginCombo("Depth Mode", depthModes[pipeline->pipelineConfigInfo.depthStencilInfo.depthTestEnable]))
			{
				for (int i = 0; i < depthModes.size(); ++i)
				{
					const bool is_selected = (pipeline->pipelineConfigInfo.depthStencilInfo.depthTestEnable == i);
					if (ImGui::Selectable(depthModes[i], is_selected))
					{
						pipeline->pipelineConfigInfo.depthStencilInfo.depthTestEnable = static_cast<VkBool32>(i);
						pipeline->pipelineConfigInfo.renderPass = Renderer::GetSwapChainRenderPass();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
        }

        if(ImGui::TreeNode("Blending"))
        {
            ImGui::TreePop();
			if (ImGui::BeginCombo("Blend Operation", blendOperations[pipeline->pipelineConfigInfo.colorBlendInfo.logicOp]))
			{
				for (int i = 0; i < blendOperations.size(); ++i)
				{
					const bool is_selected = (pipeline->pipelineConfigInfo.colorBlendInfo.logicOp == i);
					if (ImGui::Selectable(blendOperations[i], is_selected))
					{
						pipeline->pipelineConfigInfo.colorBlendInfo.logicOp = static_cast<VkLogicOp>(i);
						pipeline->pipelineConfigInfo.renderPass = Renderer::GetSwapChainRenderPass();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Source Color Blend Factor", blendFactor[pipeline->pipelineConfigInfo.colorBlendAttachments[0].srcColorBlendFactor]))
			{
				for (int i = 0; i < blendFactor.size(); ++i)
				{
					const bool is_selected = (pipeline->pipelineConfigInfo.colorBlendAttachments[0].srcColorBlendFactor == i);
					if (ImGui::Selectable(blendFactor[i], is_selected))
					{
						pipeline->pipelineConfigInfo.colorBlendAttachments[0].srcColorBlendFactor = static_cast<VkBlendFactor>(i);
						pipeline->pipelineConfigInfo.renderPass = Renderer::GetSwapChainRenderPass();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Destination Color Blend Factor", blendFactor[pipeline->pipelineConfigInfo.colorBlendAttachments[0].dstColorBlendFactor]))
			{
				for (int i = 0; i < blendFactor.size(); ++i)
				{
					const bool is_selected = (pipeline->pipelineConfigInfo.colorBlendAttachments[0].dstColorBlendFactor == i);
					if (ImGui::Selectable(blendFactor[i], is_selected))
					{
						pipeline->pipelineConfigInfo.colorBlendAttachments[0].dstColorBlendFactor = static_cast<VkBlendFactor>(i);
						pipeline->pipelineConfigInfo.renderPass = Renderer::GetSwapChainRenderPass();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Source Alpha Blend Factor", blendFactor[pipeline->pipelineConfigInfo.colorBlendAttachments[0].srcAlphaBlendFactor]))
			{
				for (int i = 0; i < blendFactor.size(); ++i)
				{
					const bool is_selected = (pipeline->pipelineConfigInfo.colorBlendAttachments[0].srcAlphaBlendFactor == i);
					if (ImGui::Selectable(blendFactor[i], is_selected))
					{
						pipeline->pipelineConfigInfo.colorBlendAttachments[0].srcAlphaBlendFactor = static_cast<VkBlendFactor>(i);
						pipeline->pipelineConfigInfo.renderPass = Renderer::GetSwapChainRenderPass();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}


			if (ImGui::BeginCombo("Destination Alpha Blend Factor", blendFactor[pipeline->pipelineConfigInfo.colorBlendAttachments[0].dstAlphaBlendFactor]))
			{
				for (int i = 0; i < blendFactor.size(); ++i)
				{
					const bool is_selected = (pipeline->pipelineConfigInfo.colorBlendAttachments[0].dstAlphaBlendFactor == i);
					if (ImGui::Selectable(blendFactor[i], is_selected))
					{
						pipeline->pipelineConfigInfo.colorBlendAttachments[0].dstAlphaBlendFactor = static_cast<VkBlendFactor>(i);
						pipeline->pipelineConfigInfo.renderPass = Renderer::GetSwapChainRenderPass();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
        }

        if(ImGui::Button("Recreate", {100, 25}))
        {
            GLTFRenderer::UpdatePipeline(type);
        }
        ImGui::PopID();
    }

    void Application::Run()
	{
        // create rendering systems
		GLTFRenderer::Init(Renderer::GetSwapChainRenderPass());

        // add functions to editor layer
        {
 			EditorLayer::AddFunction([&]() {
                ImGui::Begin("Statistics");
                ImGui::Text("Entity Count: %d", m_Scene->GetEntityCount());
				ImGui::Text("Selected Entity %d", EditorLayer::GetSelectedEntity());
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                ImGui::End();
                });

            EditorLayer::AddFunction([&]() {
                ImGui::Begin("Physics");
                ImGui::Checkbox("Enable Physics", &m_PhysicsEngine.enable);
                ImGui::DragFloat2("BoxEdges", &m_PhysicsEngine.edges.x);
                ImGui::DragFloat("Gravity", &m_PhysicsEngine.gravity, 0.1, -1.0f, 1.0f);
                ImGui::End();
                });

            EditorLayer::AddFunction([&]() {
                    ImGui::Begin("Scene Settings");
                    ImGui::Text("SkyMap");
                    ImGui::DragFloat("Exposure", &GLTFRenderer::s_SceneInfo.shaderValuesParams.exposure, 0.1f, 0.0f, 10.0f);
                    ImGui::DragFloat("Gamma", &GLTFRenderer::s_SceneInfo.shaderValuesParams.gamma, 0.1f, 0.0f, 10.0f);
                    ImGui::DragFloat("lod", &GLTFRenderer::s_SceneInfo.shaderValuesParams.lod, 0.1f, 0.0f, 10.0f);
                    ImGui::DragFloat3("Light Direction", &GLTFRenderer::s_SceneInfo.shaderValuesParams.lightDir.x);
                    if(ImGui::BeginCombo("Environment", GLTFRenderer::s_EnvMapFile.c_str()))
                    {
                        const std::string path = GetProject()->GetAssetPath() + "/environments/";
                        static const std::string ext = ".ktx";

						std::vector maps = { GLTFRenderer::s_EnvMapFile };
						for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
						{
							if (entry.path().extension() == ext)
							{
								maps.push_back(entry.path().filename().string());
							}
						}

						static int current_item = 0;
						for (int n = 0; n < maps.size(); n++)
						{
							const bool is_selected = (current_item == n);
							if (ImGui::Selectable(maps[n].data(), is_selected))
								current_item = n;
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();

                    	if(current_item != 0)
                    	{
                    		GLTFRenderer::s_EnvMapFile = path + maps[current_item];
							GLTFRenderer::UpdateScene();
                            current_item = 0;
                    	}
                    }

                    static std::vector<const char*> debugViews = { "None", "Base color", "Normal", "Occlusion", "Emissive", "Metallic", "Roughness" };
                    static int debugViewIndex = 0;
                    if (ImGui::Combo("Debug View", &debugViewIndex, &debugViews[0], debugViews.size(), debugViews.size()))
                        GLTFRenderer::s_SceneInfo.shaderValuesParams.debugViewInputs = static_cast<float>(debugViewIndex);

                    static std::vector<const char*> pbrEquations = { "None", "Diff (l,n)", "F (l,h)", "G (l,v,h)", "D (h)", "Specular" };
                    static int pbrIndex = 0;
                    if (ImGui::Combo("PBR Equation", &pbrIndex, &pbrEquations[0], pbrEquations.size(), pbrEquations.size()))
                        GLTFRenderer::s_SceneInfo.shaderValuesParams.debugViewEquation = static_cast<float>(pbrIndex);

                    ImGui::End();
                });

            EditorLayer::AddFunction([&]()
				{
                    ImGui::Begin("Pipeline");
                    AddPipelineConfigUI("PBR", GLTFRenderer::Pipes.pbr, PipelineType::PBR);
                    ImGui::Separator();
                    AddPipelineConfigUI("Skybox", GLTFRenderer::Pipes.skybox, PipelineType::SKYBOX);
                    ImGui::End();
				});

			EditorLayer::AddFunction([&]()
				{
					ImGui::Begin("Debug");
					static int entityCount = 0;
					ImGui::DragInt("Entities to add", &entityCount, 0, 100000);
					if(ImGui::SmallButton("Add"))
					{
						for(int i = 0; i < entityCount; i++) {
							m_Scene->CreateEntity("Entity");
						}
					}

					ImGui::End();
				});
        }

    	auto currentTime = std::chrono::high_resolution_clock::now();

#if 1
        for(int i = 0; i < 1; i++)
        {
			auto model = m_Scene->CreateEntity("Model");
            auto& transform = m_Scene->GetComponent<TransformComponent>(model);
			transform.translation.z = -5.0f;
			m_Scene->AddComponent<RigidBody>(model);
            m_Scene->AddComponent<Model>(model, "/models/microphone/scene.gltf");
        }
#endif

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
            GLTFRenderer::Render();

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

    		GLTFRenderer::OnUpdate();
    	}

    	vkDeviceWaitIdle(m_Device.device());
    }
} // namespace Nyxis