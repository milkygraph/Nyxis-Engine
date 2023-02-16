#include "pointLightSystem.hpp"
#include "path.hpp"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace ve
{
    struct PointLightPushConstants
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(
        VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem()
    {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");

    }

    void PointLightSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        vePipeline::defaultPipelineConfigInfo(pipelineConfig);

        // clear prevoiiusly set values
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<vePipeline>(
            current_path + "/../shaders/point_light.vert.spv",
            current_path + "/../shaders/point_light.frag.spv",
            pipelineConfig);
    }

    void PointLightSystem::render(FrameInfo &frameInfo)
    {
        pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        for(auto& kv : frameInfo.gameObjects)
        {
            if(kv.second.pointLight == nullptr) continue;

            PointLightPushConstants push{};
            push.position = glm::vec4(kv.second.transform.translation, 1.0f);
            push.color = glm::vec4(kv.second.color, 1.0f);
            push.radius = kv.second.transform.scale.x;

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);

            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }

    void PointLightSystem::Update(FrameInfo &frameInfo, GlobalUbo &ubo)
    {
        auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, {0.f, -1.f, 0.f});
        int lightIndex = 0;

        for(auto& kv: frameInfo.gameObjects)
        {
            if(kv.second.pointLight == nullptr) continue;
            
            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

            kv.second.transform.translation = glm::vec3(rotateLight * glm::vec4(kv.second.transform.translation, 1.0f));
            ubo.pointLights[lightIndex].position = glm::vec4(kv.second.transform.translation, 1.0f);
            ubo.pointLights[lightIndex].color = glm::vec4(kv.second.color, 1.0f);
            
            lightIndex++;
        }
        ubo.pointLightCount = lightIndex;
    }
} // namespace ve