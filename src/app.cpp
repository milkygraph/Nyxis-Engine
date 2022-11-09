
#include "app.hpp"
#include "model.hpp"
#include "swap_chain.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>


namespace ve
{
    struct SimplePushConstantData
    {
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    App::App()
    {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    App::~App()
    {
        vkDestroyPipelineLayout(pDevice.device(), pipelineLayout, nullptr);
    }

    void App::run()
    {
        while (!pWindow.shouldClose())
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(pDevice.device());
    }

    void App::loadModels()
    {
        std::vector<veModel::Vertex> vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

        pModel = std::make_unique<veModel>(pDevice, vertices);
    }

    void App::createPipelineLayout()
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

    void App::createPipeline()
    {
        PipelineConfigInfo pipelineConfig{};
        vePipeline::defaultPipelineConfigInfo(pipelineConfig, pSwapChain->width(), pSwapChain->height());
        pipelineConfig.renderPass = pSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        pPipeline = std::make_unique<vePipeline>(
            pDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void App::recreateSwapChain()
    {
        auto extent = pWindow.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = pWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(pDevice.device());
        if (pSwapChain == nullptr)
        {
            pSwapChain = std::make_unique<veSwapChain>(pDevice, extent);
        }
        else
        {
            pSwapChain = std::make_unique<veSwapChain>(pDevice, extent, std::move(pSwapChain));
            if (pSwapChain->imageCount() != commandBuffers.size())
            {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        createPipeline();
    }

    void App::createCommandBuffers()
    {
        commandBuffers.resize(pSwapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = pDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(pDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffer!");
        }
    }

    void App::freeCommandBuffers()
    {
        vkFreeCommandBuffers(pDevice.device(), pDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void App::recordCommandBuffer(int ImageIndex)
    {
        static int frame = 0;
        frame = (frame + 1) % 300;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[ImageIndex], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pSwapChain->getRenderPass();
        renderPassInfo.framebuffer = pSwapChain->getFrameBuffer(ImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = pSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0f, 0.0f, 0.007f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};


        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[ImageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(pSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(pSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0,0}, pSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[ImageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[ImageIndex], 0, 1, &scissor);

        pPipeline->bind(commandBuffers[ImageIndex]);
        pModel->bind(commandBuffers[ImageIndex]);

        for(int i = 0; i < 5; i++)
        {
            SimplePushConstantData push{};
            push.offset = {-2.0f + frame * 0.02f, -0.4 + i * 0.2};
            push.color = {1.0f, 0.05 + 0.1f * i, 0.01f};
            
            vkCmdPushConstants(commandBuffers[ImageIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            pModel->draw(commandBuffers[ImageIndex]);

        }


        vkCmdEndRenderPass(commandBuffers[ImageIndex]);

        if (vkEndCommandBuffer(commandBuffers[ImageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer");
        }
    }

    void App::drawFrame()
    {
        uint32_t imageIndex;
        auto result = pSwapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        recordCommandBuffer(imageIndex);

        result = pSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || pWindow.windowResized())
        {
            pWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
    } 
} // namespace ve