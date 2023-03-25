#include "NyxisUI/Viewport.hpp"
#include "Core/SwapChain.hpp"
#include "Events/MouseEvents.hpp"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>

namespace Nyxis
{
	Viewport::Viewport()
	{
		m_DescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for(auto& descriptorSet : m_DescriptorSets)
			descriptorSet = VK_NULL_HANDLE;

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(Device::get().device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create viewport sampler!");
		}
	}

	Viewport::~Viewport()
	{}

	void Viewport::OnUpdate(FrameInfo& frameInfo, VkImageView imageView)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		// ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0)); // Set the alpha channel to 0.5
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

		if (m_DescriptorSets[frameInfo.frameIndex] == VK_NULL_HANDLE)
			m_DescriptorSets[frameInfo.frameIndex] = ImGui_ImplVulkan_AddTexture(m_Sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else
		{
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler = m_Sampler;
			desc_image[0].imageView = imageView;
			desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = m_DescriptorSets[frameInfo.frameIndex];
			write_desc[0].descriptorCount = 1;
			write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc[0].pImageInfo = desc_image;
			vkUpdateDescriptorSets(Device::get().device(), 1, write_desc, 0, nullptr);
		}
		float windowWidth = ImGui::GetWindowWidth();
		float windowHeight = ImGui::GetWindowHeight();

		// display image in the middle of the window with the correct aspect ratio
		ImGui::Image(m_DescriptorSets[frameInfo.frameIndex], ImVec2(windowWidth, windowHeight));

		auto windowPos = ImGui::GetWindowPos();
		// check if mouse is in window
		if (ImGui::IsWindowHovered() && Input::isMouseButtonPressed(MouseCodes::MouseButtonRight))
		{
			Input::setCursorMode(CursorMode::CursorDisabled);
			frameInfo.scene.SetCameraControl(true);
		}

		ImGui::End();
		ImGui::PopStyleVar();
		m_Extent = { static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight)};
	}
}
