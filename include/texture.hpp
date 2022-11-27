#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stbimage/stb_image.h"
#include "frameInfo.hpp"
//createImageViews gonna be implemented to texture.cpp later
//cleanup for the createTextureImageView and CreatTextureSampler gonna be implemented later
namespace ve{
    class veTexture
    {
        // Vulkan Texture Class
        // This class is used to load textures into the Vulkan API
        // It is used to load textures from files and create Vulkan textures from them

        public:
            veTexture();
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