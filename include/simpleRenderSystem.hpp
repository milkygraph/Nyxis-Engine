#pragma once
#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "model.hpp"
#include "gameObject.hpp"
#include "renderer.hpp"
#include "simpleRenderSystem.hpp"
#include "Camera.hpp"
#include "frameInfo.hpp"
#include "scene.hpp"

#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "vepch.hpp"

namespace ve
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout);
        ~SimpleRenderSystem();

        // copy constructor and destructors

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;
        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
        void createPipeline(VkRenderPass renderPass);

        veDevice &pDevice = veDevice::get();

        std::unique_ptr<vePipeline> pPipeline;
        VkPipelineLayout pipelineLayout;
    }; // class SimpleRenderSystem
} // namespace ve
