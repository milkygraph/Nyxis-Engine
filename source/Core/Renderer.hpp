#pragma once
#include "Core/Window.hpp"
#include "Core/Device.hpp"
#include "Core/SwapChain.hpp"
#include "Scene/Scene.hpp"

namespace Nyxis
{
    class Renderer
    {
    public:
        static void Init(Window* window, Device* device);
        static void Shutdown();

    	static [[nodiscard]] VkImageView GetWorldImageView(int index);
    	static [[nodiscard]] VkRenderPass GetSwapChainRenderPass() { return m_SwapChain->GetMainRenderPass(); }
        static [[nodiscard]] VkRenderPass GetUIRenderPass() { return m_SwapChain->GetUIRenderPass(); }
        static [[nodiscard]] VkExtent2D GetAspectRatio() { return m_WorldImageSize; }
        static [[nodiscard]] VkCommandBuffer GetMainCommandBuffer();
        static [[nodiscard]] VkCommandBuffer GetUICommandBuffer();
        static [[nodiscard]] int GetFrameIndex() { return m_CurrentImageIndex; }
        static [[nodiscard]] bool IsFrameInProgress() { return m_IsFrameStarted; }
		static void SetWorldImageSize(VkExtent2D extent) { m_WorldImageSize = extent; }

        static [[nodiscard]] VkCommandBuffer BeginWorldFrame() ;
        static void EndWorldFrame();

        static [[nodiscard]] VkCommandBuffer BeginUIFrame() ;
		static void EndUIRenderPass(VkCommandBuffer commandBuffer);

    	static void BeginMainRenderPass(VkCommandBuffer commandBuffer);
        static void EndMainRenderPass(VkCommandBuffer commandBuffer);

    private:
        static void CreateCommandBuffers();
        static void FreeCommandBuffers();
        static void RecreateSwapChain();

		static inline Window* m_Window = nullptr;
        static inline Device *m_Device = nullptr;
        static inline Ref<Scene> m_Scene = nullptr;
        static inline Ref<SwapChain> m_SwapChain = nullptr;

    	static inline uint32_t m_CurrentImageIndex{};
        static inline int m_CurrentFrameIndex{};
        static inline bool m_IsFrameStarted{ false };

        static inline std::vector<VkCommandBuffer> m_MainCommandBuffers;
        static inline std::vector<VkCommandBuffer> m_UICommandBuffers;

        static inline VkExtent2D m_WorldImageSize;
		static inline VkExtent2D m_OldWorldImageSize;
    }; // class Renderer
} // namespace Nyxis
