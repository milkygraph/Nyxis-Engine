#pragma once

#include "vepch.hpp"
#include "frameInfo.hpp"
#include "descriptors.hpp"
#include "renderer.hpp"
#include "scene.hpp"

namespace ve
{
    class ImguiLayer
    {
    public:
        ImguiLayer();
        ~ImguiLayer();

        void init(VkRenderPass RenderPass, VkCommandBuffer commandBuffer);
        void OnUpdate(FrameInfo &frameInfo);
        void AddFunction(std::function<void()> func);
        void AddEntityLoader(Scene& scene);

    private:
        std::vector<std::function<void()>> functions;
        std::unique_ptr<veDescriptorPool> imguiPool{};
    };
}