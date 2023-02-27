#pragma once
#include "device.hpp"
#include "buffer.hpp"
#include "descriptors.hpp"
#include "swap_chain.hpp"

#include <vulkan/vulkan_core.h>
#include <json/json.hpp>

namespace Nyxis{

	class _Texture
	{
	public:
		explicit _Texture(const std::string& filepath);
		~_Texture();

		_Texture(const _Texture&) = delete;
		_Texture& operator=(const _Texture&) = delete;
		_Texture(_Texture&&) = delete;
		_Texture& operator=(_Texture&&) = delete;

		void ChangeImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		VkDescriptorImageInfo GetDescriptorImageInfo() const { return descriptorImageInfo; }

		void ToJson(nlohmann::json& j) const
		{
			j["Texture"] = {
				{"filepath", filepath}
			};
		}
	private:
          Device & device = Device::get();
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