#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Device.hpp"
#include "Core/FrameInfo.hpp"
#include "Core/Pipeline.hpp"

namespace Nyxis
{
    class PointLightSystem
    {
    public:
        PointLightSystem(
			VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem &) = delete;
        PointLightSystem &operator=(const PointLightSystem &) = delete;

        void Render(FrameInfo &frameInfo);
        void Update(FrameInfo &frameInfo, GlobalUbo &ubo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        Device &device = Device::get();

        std::unique_ptr<vePipeline> pipeline;
        VkPipelineLayout pipelineLayout;
    };
} // namespace Nyxis