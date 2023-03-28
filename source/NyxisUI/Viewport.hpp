#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/FrameInfo.hpp"

namespace Nyxis
{
	// imgui only viewport class
	class Viewport
	{
	public:
		Viewport();
		~Viewport();

		void OnUpdate();
		void OnEvent();
		VkExtent2D GetExtent() const { return m_Extent; }

	private:
		std::vector<VkDescriptorSet> m_DescriptorSets;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VkExtent2D m_Extent{};

	};
}
