#pragma once
#include "Core/Device.hpp"
#include "Core/Descriptors.hpp"

namespace Nyxis{
	class Texture {
	public:
		VkImage m_Image = VK_NULL_HANDLE;
		VkImageLayout m_ImageLayout;
		VkDeviceMemory m_DeviceMemory;
		VkImageView m_View;
		VkSampler m_Sampler;
		VkDescriptorImageInfo m_Descriptor;

		uint32_t m_Width{}, m_Height{};
		uint32_t m_MipLevels{};
		uint32_t m_LayerCount{};
		
		void UpdateDescriptor()
		{
			m_Descriptor.sampler = m_Sampler;
			m_Descriptor.imageView = m_View;
			m_Descriptor.imageLayout = m_ImageLayout;
		}

		void Destroy()
		{
			auto& device = Device::Get();
			vkDestroyImageView(device.device(), m_View, nullptr);
			vkDestroyImage(device.device(), m_Image, nullptr);
			if (m_Sampler)
			{
				vkDestroySampler(device.device(), m_Sampler, nullptr);
			}
			vkFreeMemory(device.device(), m_DeviceMemory, nullptr);
		}
	};


	class Texture2D : public Texture {
	public:
		void LoadFromFile(
			std::string filename,
			VkFormat format,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		void LoadFromBuffer(
			void* buffer,
			VkDeviceSize bufferSize,
			VkFormat format,
			uint32_t width,
			uint32_t height,
			VkFilter filter = VK_FILTER_LINEAR,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	private:
		void LoadFromKTXFile(
			std::string filename,
			VkFormat format,
			VkImageUsageFlags imageUsageFlags,
			VkImageLayout imageLayout);

		void LoadFromSTBFile(
			std::string filename,
			VkFormat format,
			VkImageUsageFlags imageUsageFlags,
			VkImageLayout imageLayout);
	};

	class TextureCubeMap : public Texture {
	public:
		void LoadFromFile(
			std::string filename,
			VkFormat format,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};
}
