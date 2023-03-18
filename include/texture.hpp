#pragma once
#include "device.hpp"
#include "buffer.hpp"
#include "descriptors.hpp"
#include "swap_chain.hpp"

#include <vulkan/vulkan_core.h>

#include "gli/load.hpp"
#include "gli/texture2d.hpp"
#include "gli/texture_cube.hpp"

namespace Nyxis{
	class Texture {
	public:
		Device& device = Device::get();
		VkImage m_Image = VK_NULL_HANDLE;
		VkImageLayout m_ImageLayout;
		VkDeviceMemory m_DeviceMemory;
		VkImageView m_View;
		uint32_t m_Width, m_Height;
		uint32_t m_MipLevels;
		uint32_t m_LayerCount;
		VkDescriptorImageInfo m_Descriptor;
		VkSampler m_Sampler;

		void UpdateDescriptor()
		{
			m_Descriptor.sampler = m_Sampler;
			m_Descriptor.imageView = m_View;
			m_Descriptor.imageLayout = m_ImageLayout;
		}

		void Destroy()
		{
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
		Texture2D();

		Texture2D(const Texture2D& other);

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
	};

	class TextureCubeMap : public Texture {
	public:
		TextureCubeMap& operator=(const TextureCubeMap& other)
		{
			// Self-assignment check
			if (this == &other) {
				return *this;
			}

			// Destroy the current resources of this object
			Destroy();

			// Copy the resources from the other object
			m_Image = other.m_Image;
			m_ImageLayout = other.m_ImageLayout;
			m_DeviceMemory = other.m_DeviceMemory;
			m_View = other.m_View;
			m_Width = other.m_Width;
			m_Height = other.m_Height;
			m_MipLevels = other.m_MipLevels;
			m_LayerCount = other.m_LayerCount;
			m_Descriptor = other.m_Descriptor;
			m_Sampler = other.m_Sampler;

			return *this;
		}
		void LoadFromFile(
			std::string filename,
			VkFormat format,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};
}
