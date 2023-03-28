#pragma once
#include "Core/Pipeline.hpp"
#include "Core/Device.hpp"
#include "Core/FrameInfo.hpp"
#include "RenderSystems/SimpleRenderSystem.hpp"

namespace Nyxis
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout);
        ~SimpleRenderSystem();

        // copy constructor and destructors

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;
        void Render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
        void createPipeline(VkRenderPass renderPass);

        Device &pDevice = Device::Get();

        std::unique_ptr<vePipeline> pPipeline;
        VkPipelineLayout pipelineLayout;
    }; // class SimpleRenderSystem
} // namespace Nyxis
