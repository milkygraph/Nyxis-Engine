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

    	[[nodiscard]] static VkImageView GetWorldImageView(int index);
    	[[nodiscard]] static VkRenderPass GetSwapChainRenderPass() { return m_SwapChain->GetMainRenderPass(); }
        [[nodiscard]] static VkRenderPass GetUIRenderPass() { return m_SwapChain->GetUIRenderPass(); }
        [[nodiscard]] static VkExtent2D GetAspectRatio() { return m_WorldImageSize; }
        [[nodiscard]] static VkCommandBuffer GetMainCommandBuffer();
        [[nodiscard]] static VkCommandBuffer GetUICommandBuffer();
        [[nodiscard]] static int GetFrameIndex() { return m_CurrentImageIndex; }
        [[nodiscard]] static bool IsFrameInProgress() { return m_IsFrameStarted; }
		static void SetWorldImageSize(VkExtent2D extent) { m_WorldImageSize = extent; }

        [[nodiscard]]  static VkCommandBuffer BeginWorldFrame() ;
        static void EndWorldFrame();

        [[nodiscard]] static VkCommandBuffer BeginUIFrame() ;
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
