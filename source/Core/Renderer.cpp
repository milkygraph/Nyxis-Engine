#include "Core/Renderer.hpp"
#include "Core/Nyxispch.hpp"

namespace Nyxis
{
    void Renderer::Init(Window* window, Device* device)
    {
	    LOG_INFO("[Core] Initializing Renderer");
        m_Window = window;
        m_Device = device;
    	RecreateSwapChain();
        CreateCommandBuffers();
        m_WorldImageSize = m_SwapChain->GetSwapChainExtent();
        m_OldWorldImageSize = m_WorldImageSize;
    }

    void Renderer::Shutdown()
    {
        FreeCommandBuffers();
    }

    VkImageView Renderer::GetWorldImageView(int index)
    {
        return m_SwapChain->GetWorldImageView(index);
    }

    void Renderer::RecreateSwapChain()
    {
	    auto windowExtent = m_Window->GetExtent();
        while (windowExtent.width == 0 || windowExtent.height == 0)
        {
            windowExtent = m_Window->GetExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_Device->device());
        if (m_SwapChain == nullptr)
        {
            m_SwapChain = std::make_unique<SwapChain>(windowExtent);
        }
        else
        {
            m_SwapChain = std::make_unique<SwapChain>(windowExtent, m_WorldImageSize, std::move(m_SwapChain));
            if (m_SwapChain->ImageCount() != m_MainCommandBuffers.size())
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
        allocInfo.commandPool = m_Device->getCommandPool({ World });
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_MainCommandBuffers.size());

        if (vkAllocateCommandBuffers(m_Device->device(), &allocInfo, m_MainCommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffer!");
        }

        m_UICommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        allocInfo.commandPool = m_Device->getCommandPool({ Final });
        if (vkAllocateCommandBuffers(m_Device->device(), &allocInfo, m_UICommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffer!");
        }
    }

    void Renderer::FreeCommandBuffers()
    {
	    vkFreeCommandBuffers(m_Device->device(), m_Device->getCommandPool( {World} ), static_cast<uint32_t>(m_MainCommandBuffers.size()), m_MainCommandBuffers.data());
		vkFreeCommandBuffers(m_Device->device(), m_Device->getCommandPool( {Final} ), static_cast<uint32_t>(m_UICommandBuffers.size()), m_UICommandBuffers.data());
    	m_MainCommandBuffers.clear();
		m_UICommandBuffers.clear();
    }

    VkCommandBuffer Renderer::GetMainCommandBuffer()
    {
        assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
        return m_MainCommandBuffers[m_CurrentImageIndex];
    }

    VkCommandBuffer Renderer::GetUICommandBuffer()
    {
        assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
        return m_UICommandBuffers[m_CurrentImageIndex];
    }


    VkCommandBuffer Renderer::BeginWorldFrame()
    {
        assert(!m_IsFrameStarted && "Can't call BeginWorldFrame while already in progress");

        auto result = m_SwapChain->AcquireNextImage(&m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return VK_NULL_HANDLE;
        }

		if (m_WorldImageSize.width != m_OldWorldImageSize.width || m_WorldImageSize.height != m_OldWorldImageSize.height)
		{
			m_SwapChain->SetWorldImageExtent(m_WorldImageSize);
			m_SwapChain->RecreateWorldImages();
			m_OldWorldImageSize = m_WorldImageSize;
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
        renderPassInfo.renderPass = m_SwapChain->GetUIRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->GetSwapChainFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

        VkClearValue clearValues;
        clearValues = { 0.1f, 0.1f, 0.1f, 1.0f };

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, m_SwapChain->GetSwapChainExtent() };
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

        auto result = m_SwapChain->SubmitSwapChainCommandBuffers(commandBuffers, &m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window->WindowResized())
        {
            m_Window->ResetWindowResizedFlag();
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
        renderPassInfo.renderPass = m_SwapChain->GetMainRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->GetWorldFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_SwapChain->GetWorldExtent();

        std::array<VkClearValue, 3> clearValues{};
        clearValues[0].color = {.0f, .0f, .0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    	VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain->GetWorldExtent().width);
        viewport.height = static_cast<float>(m_SwapChain->GetWorldExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_SwapChain->GetWorldExtent()};
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
        m_SwapChain->SubmitWorldCommandBuffers(&worldCommandBuffer, &m_CurrentImageIndex);
	}
} // namespace Nyxis