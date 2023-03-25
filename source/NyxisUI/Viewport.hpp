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

		void OnUpdate(FrameInfo& frameInfo, VkImageView imageView);
		VkExtent2D GetExtent() const { return m_Extent; }

	private:
		std::vector<VkDescriptorSet> m_DescriptorSets;
		VkSampler m_Sampler;
		VkExtent2D m_Extent{};

	};
}
