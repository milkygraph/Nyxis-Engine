#include "texture.hpp"
#include <vulkan/vulkan_core.h>

namespace ve
{
    veTexture::veTexture()
    {

    }

    veTexture::~veTexture()
    {

    }

    void veTexture::loadTexture(std::string& filename, veDevice& device)
    {
        int texWidth, texHeight, texChannels;
        // stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        // if (!pixels)
            // throw std::runtime_error("failed to load texture image!");
    }
}