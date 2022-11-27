#pragma once

#include "stbimage/stb_image.h"
#include "frameInfo.hpp"

namespace ve{
class veTexture
{
    // Vulkan Texture Class
    // This class is used to load textures into the Vulkan API
    // It is used to load textures from files and create Vulkan textures from them

    public:
        veTexture();
        ~veTexture();

        void loadTexture(std::string& filename, veDevice& device);
        void createTextureImageView(veDevice& device);
        void createTextureSampler(VkDevice& device);

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
};
}