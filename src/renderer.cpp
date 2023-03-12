#include "renderer.hpp"
#include "model.hpp"
#include "Nyxispch.hpp"

namespace Nyxis
{
    Renderer::Renderer(Scene& scene)
    {
        this->scene = &scene;
        RecreateSwapChain();
        CreateCommandBuffers();
    }

    Renderer::~Renderer()
    {
        FreeCommandBuffers();
    }

    VkImageView Renderer::GetWorldImageView(int index) const
    {
        return pSwapChain->GetWorldImageView(index);
    }

    void Renderer::RecreateSwapChain()
    {
	    auto extent = window.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device.device());
        if (pSwapChain == nullptr)
        {
            pSwapChain = std::make_unique<SwapChain>(extent);
        }
        else
        {
            pSwapChain = std::make_unique<SwapChain>(extent, std::move(pSwapChain));
            if (pSwapChain->ImageCount() != m_MainCommandBuffers.size())
            {
                FreeCommandBuffers();
                CreateCommandBuffers();
            }
        }
    }

    void Renderer::CreateCommandBuffers()
    {
        m_MainCommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.getCommandPool({ World });
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_MainCommandBuffers.size());

        if (vkAllocateCommandBuffers(device.device(), &allocInfo, m_MainCommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffer!");
        }

        m_UICommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        allocInfo.commandPool = device.getCommandPool({ Final });
        if (vkAllocateCommandBuffers(device.device(), &allocInfo, m_UICommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffer!");
        }
		LOG_INFO("Succesfully created main command buffers.");
    }

    void Renderer::FreeCommandBuffers()
    {
	    vkFreeCommandBuffers(device.device(), device.getCommandPool( {World} ), static_cast<uint32_t>(m_MainCommandBuffers.size()), m_MainCommandBuffers.data());
		vkFreeCommandBuffers(device.device(), device.getCommandPool( {Final} ), static_cast<uint32_t>(m_UICommandBuffers.size()), m_UICommandBuffers.data());
    	m_MainCommandBuffers.clear();
		m_UICommandBuffers.clear();
    }

    VkCommandBuffer Renderer::BeginWorldFrame()
    {
        assert(!m_IsFrameStarted && "Can't call BeginWorldFrame while already in progress");

        auto result = pSwapChain->AcquireNextImage(&m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return VK_NULL_HANDLE;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        m_IsFrameStarted = true;
        auto commandBuffer = GetMainCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");

		return commandBuffer;
    }

    void Renderer::EndWorldFrame()
	{
	}

	VkCommandBuffer Renderer::BeginUIFrame()
	{
		auto commandBuffer = GetUICommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer!");

        assert(m_IsFrameStarted && "Can't call BeginSwapChainRenderPass while in progress");
        assert(commandBuffer == GetUICommandBuffer() && "Can't begin render pass on command buffer from another frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pSwapChain->GetUIRenderPass();
        renderPassInfo.framebuffer = pSwapChain->GetSwapChainFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = pSwapChain->GetSwapChainExtent();

        VkClearValue clearValues;
        clearValues = { .3f, .3f, .3f, 1.0f };

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(pSwapChain->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(pSwapChain->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, pSwapChain->GetSwapChainExtent() };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		return commandBuffer;
	}

    void Renderer::EndUIRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(m_IsFrameStarted && "Can't call EndUIRenderPass while in progress");
        assert(commandBuffer == GetUICommandBuffer() && "Can't end render pass on command buffer from another frame");
    	vkCmdEndRenderPass(commandBuffer);

        VkCommandBuffer commandBuffers[] = { GetUICommandBuffer() };

        if (vkEndCommandBuffer(commandBuffers[0]) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer");

        auto result = pSwapChain->SubmitSwapChainCommandBuffers(commandBuffers, &m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.windowResized())
        {
            window.resetWindowResizedFlag();
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS)
            throw std::runtime_error("failed to present swap chain image!");

        m_IsFrameStarted = false;
    }

    void Renderer::BeginMainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(m_IsFrameStarted && "Can't call BeginMainRenderPass while in progress");
        assert(commandBuffer == GetMainCommandBuffer() && "Can't begin render pass on command buffer from another frame");
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pSwapChain->GetMainRenderPass();
        renderPassInfo.framebuffer = pSwapChain->GetWorldFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = pSwapChain->GetSwapChainExtent();

        std::array<VkClearValue, 3> clearValues{};
        clearValues[0].color = { 1.0f, .0f, .0f, 1.0f };
        clearValues[0].color = {1.0f, .0f, .0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    	VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(pSwapChain->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(pSwapChain->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, pSwapChain->GetSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Renderer::EndMainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(m_IsFrameStarted && "Can't call EndMainRenderPass while in progress");
        assert(commandBuffer == GetMainCommandBuffer() && "Can't end render pass on command buffer from another frame");

        vkCmdEndRenderPass(commandBuffer);

    	assert(m_IsFrameStarted && "Can't end frame while not in progress ");

        auto worldCommandBuffer = GetMainCommandBuffer();

        if (vkEndCommandBuffer(worldCommandBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer");
        pSwapChain->SubmitWorldCommandBuffers(&worldCommandBuffer, &m_CurrentImageIndex);
	}
} // namespace Nyxis