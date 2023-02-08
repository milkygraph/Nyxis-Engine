#include "app.hpp"
#include "components.hpp"
#include "descriptors.hpp"
#include "gameObject.hpp"
#include "model.hpp"
#include "renderer.hpp"
#include "swap_chain.hpp"
#include "Camera.hpp"
#include "CameraController.hpp"
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

        // calculate the time it takes for below code to execute
        auto start = std::chrono::high_resolution_clock::now();
        loadGameObjects();
        pScene.loadModels();
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

    void App::run() {
        std::vector<std::unique_ptr<veBuffer>> uboBuffers(veSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &uboBuffer: uboBuffers) {
            uboBuffer = std::make_unique<veBuffer>(
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffer->map();
        }

        auto globalSetLayout = veDescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(veSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            veDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(globalDescriptorSets[i]);
        }


        SimpleRenderSystem srs{pRenderer.getSwapChainRenderPass(),
                               globalSetLayout->getDescriptorSetLayout()}; // srs - simpleRenderSystem
		PointLightSystem pls{ pRenderer.getSwapChainRenderPass(),
		                      globalSetLayout->getDescriptorSetLayout() };   // pls - pointLightSystem
		TextureRenderSystem trs{ pRenderer.getSwapChainRenderPass(),
		                         globalSetLayout->getDescriptorSetLayout() }; // trs - textureRenderSystem

        Camera Camera({0, -1, -2.5});

        auto currentTime = std::chrono::high_resolution_clock::now();

        bool firstFrame = true;

        pImguiLayer.AddFunction([&]() {
            //
            ImGui::Begin("Statistics");
            ImGui::Text("Entity Count: %d", pScene.getEntityCount());
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::End();
        });

        pImguiLayer.AddFunction([&]() {
            ImGui::Begin("Camera");
            ImGui::DragFloat("Move Speed", &Camera.getCameraController().moveSpeed, 0.1f);
            ImGui::End();
        });

        pImguiLayer.AddEntityLoader(pScene);

        auto commandBuffer = pRenderer.beginFrame();
        pImguiLayer.init(pRenderer.getSwapChainRenderPass(), commandBuffer);
        pRenderer.beginSwapChainRenderPass(commandBuffer);
        pRenderer.endSwapChainRenderPass(commandBuffer);
        pRenderer.endFrame();

        while (!pWindow.shouldClose()) {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            commandBuffer = pRenderer.beginFrame();


            float aspect = pRenderer.getAspectRatio();

            Camera.setPerspectiveProjection(glm::radians(60.f), aspect, 0.01f, 1000.f);

            int frameIndex = pRenderer.getFrameIndex();
			texturePool[frameIndex]->resetPool();
            FrameInfo frameInfo
                    {frameIndex, frameTime, commandBuffer, Camera, globalDescriptorSets[frameIndex], *texturePool[frameIndex], gameObjects,
                     pScene, 0};

            // updating Camera
            Camera.OnUpdate(frameInfo.frameTime);

            // updating buffers
            GlobalUbo ubo{};
            ubo.projection = Camera.getProjectionMatrix();
            ubo.view = Camera.getViewMatrix();
            ubo.inverseViewMatrix = Camera.getInverseViewMatrix();
            pls.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // rendering
            pRenderer.beginSwapChainRenderPass(frameInfo.commandBuffer);
            srs.render(frameInfo);
            pls.render(frameInfo);
	        trs.Render(frameInfo);
            pImguiLayer.OnUpdate(frameInfo);
            pRenderer.endSwapChainRenderPass(frameInfo.commandBuffer);
            pRenderer.endFrame();
        }

        vkDeviceWaitIdle(pDevice.device());
    }

    void App::loadGameObjects()
    {
		auto pose = pScene.createEntity("Pose");
		pScene.addComponent<TransformComponent>(pose,
			glm::vec3(.2f, .5f, 0.f),
			glm::vec3(.0f, .0f, 0.0f),
			glm::vec3(1.5f, 1.5f, 1.5f),
			0.0f);
		pScene.addComponent<MeshComponent>(pose, model_path + "pose.obj");
		pScene.addComponent<Texture>(pose, "../textures/man.jpg");

	    auto floor = pScene.createEntity("Floor");
	    pScene.addComponent<TransformComponent>(floor,
		    glm::vec3(.2f, .5f, 0.f),
		    glm::vec3(.0f, .0f, 0.0f),
		    glm::vec3(1.5f, 1.5f, 1.5f),
		    0.0f);
	    pScene.addComponent<MeshComponent>(floor, model_path + "floor.obj");
	    pScene.addComponent<Texture>(floor, "../textures/pavement.jpg");


	    std::vector<glm::vec3> lightColors{
			{ 1.f, .1f, .1f },
			{ .1f, .1f, 1.f },
			{ .1f, 1.f, .1f },
			{ 1.f, 1.f, .1f },
			{ .1f, 1.f, 1.f },
			{ 1.f, 1.f, 1.f }};

		for (int i = 0; i < lightColors.size(); i++)
		{
			auto pointLight = veGameObject::makePointLight(0.8f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
    }
} // namespace ve