#include "app.hpp"
#include "components.hpp"
#include "descriptors.hpp"
#include "model.hpp"
#include "swap_chain.hpp"
#include "frameInfo.hpp"
#include "simpleRenderSystem.hpp"
#include "pointLightSystem.hpp"
#include "TextureRenderSystem.hpp"
#include "path.hpp"
#include "Log.hpp"

#include "vepch.hpp"


std::vector<std::string> getModelNames() {
    std::vector<std::string> modelNames;
    for (const auto &entry: std::filesystem::directory_iterator(ve::model_path)) {
        modelNames.push_back(entry.path().filename().string());
    }
    return modelNames;
}

namespace ve
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
			.setMaxSets(veSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, veSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, veSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		texturePool.resize(veSwapChain::MAX_FRAMES_IN_FLIGHT);
		auto framePoolBuilder = veDescriptorPool::Builder()
			.setMaxSets(1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		for (auto & pool : texturePool) {
			pool = framePoolBuilder.build();
		}

		uboBuffers.resize(veSwapChain::MAX_FRAMES_IN_FLIGHT);
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
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		globalDescriptorSets.resize(veSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			veDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		auto commandBuffer = pDevice.beginSingleTimeCommands();
		pImguiLayer.init(pRenderer.getSwapChainRenderPass(), commandBuffer);
		pDevice.endSingleTimeCommands(commandBuffer);
	}

    void App::run() {
        SimpleRenderSystem srs{pRenderer.getSwapChainRenderPass(),
                               globalSetLayout->getDescriptorSetLayout()}; // srs - simpleRenderSystem
		PointLightSystem pls{ pRenderer.getSwapChainRenderPass(),
		                      globalSetLayout->getDescriptorSetLayout() };   // pls - pointLightSystem
		TextureRenderSystem trs{ pRenderer.getSwapChainRenderPass(),
		                         globalSetLayout->getDescriptorSetLayout() }; // trs - textureRenderSystem

        auto currentTime = std::chrono::high_resolution_clock::now();

        pImguiLayer.AddFunction([&]() {
            ImGui::Begin("Statistics");
            ImGui::Text("Entity Count: %d", pScene.getEntityCount());
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::End();
        });

        pImguiLayer.AddFunction([&CameraController = pScene.GetCamera()->getCameraController()]() {
            ImGui::Begin("Camera");
            ImGui::DragFloat("Move Speed", &(CameraController.moveSpeed), 0.1f);
            ImGui::End();
        });

        pImguiLayer.AddEntityLoader(pScene);

        pImguiLayer.AddFunction([&]() {
            ImGui::Begin("Scene");
            if (ImGui::Button("Save Scene")) {
                pScene.SaveSceneFlag = true;
            }
            if(ImGui::Button("Load Scene")) {
                vkDeviceWaitIdle(pDevice.device());
                pScene.LoadSceneFlag = true;
                pImguiLayer.update = true;
            }
            ImGui::End();
        });


        while (!pWindow.shouldClose()) {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            auto commandBuffer = pRenderer.beginFrame();

            float aspect = pRenderer.getAspectRatio();

            int frameIndex = pRenderer.getFrameIndex();
			texturePool[frameIndex]->resetPool();
            FrameInfo frameInfo
                    {frameIndex, frameTime, commandBuffer, globalDescriptorSets[frameIndex], *texturePool[frameIndex], gameObjects, pScene};

            // updating Camera

            // updating buffers TODO move to scene update
            GlobalUbo ubo{};
            ubo.projection = pScene.GetCamera()->getProjectionMatrix();
            ubo.view = pScene.GetCamera()->getViewMatrix();
            ubo.inverseViewMatrix = pScene.GetCamera()->getInverseViewMatrix();
            pls.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // rendering TODO move to scene update
            pRenderer.beginSwapChainRenderPass(frameInfo.commandBuffer);
	        trs.Render(frameInfo);
            srs.render(frameInfo);
            pls.render(frameInfo);
            pImguiLayer.OnUpdate(frameInfo);
            pRenderer.endSwapChainRenderPass(frameInfo.commandBuffer);
            pRenderer.endFrame();

			pScene.OnUpdate(frameInfo.frameTime, aspect, false);
        }

        vkDeviceWaitIdle(pDevice.device());
    }

    void App::loadGameObjects()
    {
		 auto background = pScene.createEntity("Background");
	     pScene.addComponent<TransformComponent>( background,
		     glm::vec3(0.f, 0.f, 0.f),
		     glm::vec3(.0f, .0f, 0.0f),
		     glm::vec3(0.1f, 0.1f, 0.1f),
		     0.0f);
		 pScene.addComponent<MeshComponent>(background, model_path + "background.obj");
		 pScene.addComponent<Texture>(background, texture_path + "pngegg.png");
         pScene.addComponent<Player>(background);
//        pScene.LoadScene("scene.json");

	}
} // namespace ve