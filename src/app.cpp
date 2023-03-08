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
#include "AssimpModel.hpp"

#include "Nyxispch.hpp"


std::vector<std::string> getModelNames() {
    std::vector<std::string> modelNames;
    for (const auto &entry: std::filesystem::directory_iterator(Nyxis::model_path)) {
        modelNames.push_back(entry.path().filename().string());
    }
    return modelNames;
}

namespace Nyxis
{
    App *App::pInstance = nullptr;

    App::App() {

        // calculate the time it takes for below code to execute
        auto start = std::chrono::high_resolution_clock::now();
		Setup();
        loadGameObjects();
	    pScene.LoadModels();
        std::cout << "loadGameObjects() took " << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start).count() << "ms" << std::endl;
        pWindow.SetEventCallback(std::bind(&App::OnEvent, this, std::placeholders::_1));
        pInstance = this;
    }

    App::~App() = default;

    void App::OnEvent(Event &e) {
        std::string event_name = e.toString();
#if LOGGING_LEVEL == 0
        LOG_INFO(event_name);
#endif // LOGGING_LEVEL
    }

	void App::Setup()
	{
		globalPool = veDescriptorPool::Builder()
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		uboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (auto &uboBuffer: uboBuffers) {
			uboBuffer = std::make_unique<Buffer>(
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffer->map();
		}

		globalSetLayout = veDescriptorSetLayout::Builder()
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			veDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		auto commandBuffer = pDevice.beginSingleTimeCommands();
		pImguiLayer.init(pRenderer.GetUIRenderPass(), commandBuffer);
		pDevice.endSingleTimeCommands(commandBuffer);
	}

    void App::run() {

        // create systems
        SimpleRenderSystem srs{pRenderer.GetSwapChainRenderPass(),
                               globalSetLayout->getDescriptorSetLayout()}; // srs - simpleRenderSystem
		PointLightSystem pls{ pRenderer.GetSwapChainRenderPass(),
		                      globalSetLayout->getDescriptorSetLayout() };   // pls - pointLightSystem
		TextureRenderSystem trs{ pRenderer.GetSwapChainRenderPass(),
		                         globalSetLayout->getDescriptorSetLayout() }; // trs - textureRenderSystem
        ParticleRenderSystem prs{ pRenderer.GetSwapChainRenderPass(),
                                 globalSetLayout->getDescriptorSetLayout() }; // prs - particleRenderSystem
        {
            Particle particle;
            particle.color = { 1.0f, 0.2f, 0.3f, 1.0f };
            prs.AddParticle(particle);
            prs.BuildBuffer();
        }

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
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        
        while (!pWindow.shouldClose()) {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            float aspect = pRenderer.GetAspectRatio();
            auto commandBuffer = pRenderer.BeginWorldFrame();
            int frameIndex = pRenderer.GetFrameIndex();

            FrameInfo frameInfo
                    {frameIndex, frameTime, commandBuffer, globalDescriptorSets[frameIndex], gameObjects, pScene};

            {
            	// updating buffers TODO move to scene update
                GlobalUbo ubo{};
                ubo.UpdateVPM(pScene.GetCamera());
                pls.Update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // rendering TODO move to scene update
                pRenderer.BeginMainRenderPass(frameInfo.commandBuffer);

                prs.Render(frameInfo);
                trs.Render(frameInfo);
                srs.Render(frameInfo);
                pls.Render(frameInfo);
                if (PhysicsEnabled)
                    physicsEngine.OnUpdate(pScene, frameInfo.frameTime);

                pRenderer.EndMainRenderPass(frameInfo.commandBuffer);
                pRenderer.EndWorldFrame();
            }

            {
				pRenderer.BeginUIFrame(frameInfo);
                pRenderer.BeginUIRenderPass(frameInfo.commandBuffer);
				pImguiLayer.OnUpdate(frameInfo, pRenderer.GetWorldImageView(frameInfo.frameIndex));
                pRenderer.EndUIRenderPass(frameInfo.commandBuffer);
                pRenderer.EndUIFrame();
            }

			pRenderer.EndRender();

            pScene.OnUpdate(frameInfo.frameTime, aspect);
        }

        vkDeviceWaitIdle(pDevice.device());
    }

    void App::loadGameObjects()
    {
        auto circle1 = pScene.createEntity("Circle1");
        auto& rigidBody1 = pScene.addComponent<RigidBody>(circle1);
		rigidBody1.translation = glm::vec3(0.0f, -1.0f, 1.0f);
		rigidBody1.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        rigidBody1.scale = glm::vec3(1.0f, 1.0f, 1.0f);

        pScene.addComponent<MeshComponent>(circle1, "../models/circle.obj");
        pScene.addComponent<Collider>(circle1, ColliderType::Sphere, glm::vec3{ 0.2, 0.2, 0.2 }, 0.05);
        pScene.addComponent<Gravity>(circle1);
		pScene.addComponent<Texture>(circle1, "../textures/pavement.jpg");
        pScene.addComponent<Player>(circle1);

		auto floor = pScene.createEntity("Floor");
		auto& rigidBody2 = pScene.addComponent<RigidBody>(floor);
		rigidBody2.translation = glm::vec3(0.0f, 1.0f, 1.0f);
        rigidBody2.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		rigidBody2.scale = glm::vec3(1.0f, 1.0f, 1.0f);
        
		pScene.addComponent<MeshComponent>(floor, "../models/floor.obj");
		pScene.addComponent<Gravity>(floor);
        pScene.addComponent<Texture>(floor, "../textures/pavement.jpg");

        
        //for(auto i = 0; i < 10; i++)
        //{
        //    auto circle = pScene.createEntity("Circle" + std::to_string(i));
        //    auto& rigidBody = pScene.addComponent<RigidBody>(circle);
        //    rigidBody.translation = glm::vec3(-1.0f + i * 0.1f, -1.0f, 1.0f);
        //    rigidBody.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        //    rigidBody.scale = glm::vec3(.01f, .01f, .01f);
        //    rigidBody.restitution = 0.5f;

        //    pScene.addComponent<MeshComponent>(circle, "../models/sphere.obj");
        //    pScene.addComponent<Collider>(circle, ColliderType::Sphere, glm::vec3{ 0.2, 0.2, 0.2 }, 0.02);
        //    pScene.addComponent<Gravity>(circle);
        //}
    }
} // namespace Nyxis