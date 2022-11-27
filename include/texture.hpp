#pragma once
#include "device.hpp"
#include <vulkan/vulkan_core.h>

//createImageViews gonna be implemented to texture.cpp later
//cleanup for the createTextureImageView and CreatTextureSampler gonna be implemented later
namespace ve{
    class veTexture
    {
        // Vulkan Texture Class
        // This class is used to load textures into the Vulkan API
        // It is used to load textures from files and create Vulkan textures from them

        public:
            veTexture(veDevice &device);
            ~veTexture();

            void loadTexture(std::string& filename);
            void createTextureImageView();
            void createTextureSampler();
            void createImage();
            VkImageView createImageView();
            void createImageViews();

            VkImage textureImage;
            VkDeviceMemory textureImageMemory;
            VkImageView textureImageView;
            VkSampler textureSampler;
            int texWidth;
            int texHeight;
            int texChannels;
        private:
            veDevice& device;
    };

}