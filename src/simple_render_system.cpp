
#include "simpleRenderSystem.hpp"
#include "model.hpp"
#include "renderer.hpp"
#include "swap_chain.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>

namespace ve
{
    struct SimplePushConstantData
    {
        glm::mat4 transform{1.f};
        alignas(16) glm::vec3 color;
    };

    SimpleRenderSystem::SimpleRenderSystem(veDevice& device, VkRenderPass renderPass)
        : pDevice(device)
    {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem()
    {
        vkDestroyPipelineLayout(pDevice.device(), pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(pDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        //assert(pipelineLayout == nullptr && "Can not create pipeline before pipeline layout");
        
        PipelineConfigInfo pipelineConfig{};
        vePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pPipeline = std::make_unique<vePipeline>(
            pDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<veGameObject>& gameObjects, veCamera& camera)
    {
        pPipeline->bind(commandBuffer);

        auto projectionView = camera.getProjectionMatrx() * camera.getViewMatrix();

        for (auto &obj : gameObjects)
        {
            static bool flag = true;
            if(flag)
            {
            obj.transform.rotation.y = obj.transform.rotation.x + 1;
            flag = false;
            }
            //srand(time(NULL));
            //auto angle = static_cast<float>(std::rand() % 100 - 50) / 3000;
            obj.transform.rotation.y = obj.transform.rotation.y + 0.005;
            obj.transform.rotation.z = obj.transform.rotation.z + 0.005;
            obj.transform.rotation.x = obj.transform.rotation.x + 0.005;
            SimplePushConstantData push{};
            push.color = obj.color;
            push.transform = projectionView * obj.transform.mat4();

            vkCmdPushConstants(
                commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
} // namespace ve