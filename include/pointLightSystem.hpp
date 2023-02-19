#pragma once

#include "Camera.hpp"
#include "device.hpp"
#include "frameInfo.hpp"
#include "gameObject.hpp"
#include "pipeline.hpp"

// std
#include "vepch.hpp"

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