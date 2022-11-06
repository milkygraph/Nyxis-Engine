#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"

namespace ve
{
    class App
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void run();

    private:
        veWindow pWindow{WIDTH, HEIGHT, "Vulkan"};
        veDevice pDevice{pWindow}; 
        vePipeline pPipeline{pDevice, "shaders/simple_vertex.spv", "shaders/simple_fragment.spv", vePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };
} // namespace ve