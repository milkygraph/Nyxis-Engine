#include "texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stbimage/stb_image.h"

namespace ve
{
    veTexture::veTexture(veDevice &device)
     : device(device)
    {
        int texWidth = 0;
        int texHeight = 0;
        int texChannels = 0;
    }

    veTexture::~veTexture()
    {

    }

    void veTexture::loadTexture(std::string& filename)
    {
        
        stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels)
            throw std::runtime_error("failed to load texture image!");
    }

    void veTexture::createImage(){
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB; //This might be changed, since some hardware might not support it
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; 
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;

        if(vkCreateImage(device.device(), &imageInfo, nullptr, &textureImage)){
           throw std::runtime_error("failed to create image!");
        }

    }

    VkImageView veTexture::createImageView(){
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = textureImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.components = {};

        VkImageView imageView;
        if(vkCreateImageView(device.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS){
            throw std::runtime_error("failed to create texture image view!");
        }
        return imageView;
    }
    void veTexture::createTextureImageView(){
        textureImageView = createImageView();
    }

    void veTexture::createTextureSampler(){
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        //Effects the performance
        //samplerInfo.anisotropyEnable = VK_TRUE;
        //samplerInfo.maxAnisotropy = ???;
        VkPhysicalDeviceProperties properties = device.properties;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias= 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if(vkCreateSampler(device.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS){
            throw std::runtime_error("failed to create texture sampler!");
        }

    }

}