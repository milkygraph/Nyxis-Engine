#pragma once
#include "device.hpp"
#include "buffer.hpp"
#include "descriptors.hpp"
#include "swap_chain.hpp"

#include <vulkan/vulkan_core.h>
#include <json/json.hpp>

namespace ve{

	class Texture
	{
	public:
		explicit Texture(const std::string& filepath);
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) = delete;
		Texture& operator=(Texture&&) = delete;

		void ChangeImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		VkDescriptorImageInfo GetDescriptorImageInfo() const { return descriptorImageInfo; }

		void ToJson(nlohmann::json& j) const
		{
			j["Texture"] = {
				{"filepath", filepath}
			};
		}
	private:
		veDevice& device = veDevice::get();
		std::string filepath;
		int texWidth = 0;
		int texHeight = 0;
		int texChannels = 0;
		int bytesPerPixel = 0;
		uint32_t mipLevels = 0;

		VkImage textureImage = VK_NULL_HANDLE;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkImageView textureImageView = VK_NULL_HANDLE;
		VkSampler textureSampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo descriptorImageInfo = {};

		VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	};
}