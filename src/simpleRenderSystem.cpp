#include "simpleRenderSystem.hpp"
#include "model.hpp"
#include "renderer.hpp"
#include "swap_chain.hpp"
#include "path.hpp"
#include "components.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>

namespace ve
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
        float roughness{0.0f};
    };

    SimpleRenderSystem::SimpleRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout)
	{
        createPipelineLayout(globalDescriptorSetLayout);
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem()
    {
        vkDestroyPipelineLayout(pDevice.device(), pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalDescriptorSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(pDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        // assert(pipelineLayout == nullptr && "Can not create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        vePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pPipeline = std::make_unique<vePipeline>(
            current_path + "/../shaders/simple_shader.vert.spv",
            current_path + "/../shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void SimpleRenderSystem::render(FrameInfo &frameInfo)
    {
        pPipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                0, 1,
                                &frameInfo.globalDescriptorSet, 0, nullptr);

        auto view = frameInfo.scene.getComponentView<TransformComponent, veModel>();

		std::mutex vulkanGuard;
        for (auto& entity : view)
        {
	        auto [transform, model] = view.get<TransformComponent, veModel>(entity);
			auto name = frameInfo.scene.getComponent<TagComponent>(entity).Tag;
			if(model.loaded)
				{
					SimplePushConstantData push{};
					push.modelMatrix = transform.mat4 ();
					push.normalMatrix = transform.normalMatrix ();
					push.roughness = transform.roughness;

					std::unique_lock<std::mutex> lock(vulkanGuard);
					vkCmdPushConstants (
						frameInfo.commandBuffer,
						pipelineLayout,
						VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
						0,
						sizeof (SimplePushConstantData),
						&push);
					model.bind (frameInfo.commandBuffer);
					model.draw (frameInfo.commandBuffer);
				}
		}
    }
} // namespace ve