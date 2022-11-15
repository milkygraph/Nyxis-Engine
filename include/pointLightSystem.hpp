#pragma once

#include "camera.hpp"
#include "device.hpp"
#include "frameInfo.hpp"
#include "gameObject.hpp"
#include "pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace ve
{
    class PointLightSystem
    {
    public:
        PointLightSystem(
            veDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem &) = delete;
        PointLightSystem &operator=(const PointLightSystem &) = delete;

        void render(FrameInfo &frameInfo);
        void update(FrameInfo &frameInfo, GlobalUbo &ubo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        veDevice &device;

        std::unique_ptr<vePipeline> pipeline;
        VkPipelineLayout pipelineLayout;
    };
} // namespace ve