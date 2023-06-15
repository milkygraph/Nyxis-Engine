#pragma once
#include "Core/Layer.hpp"

namespace Nyxis
{
	class MenuBar final : public Layer
	{
	public:
		MenuBar() = default;
		~MenuBar() override = default;
		void OnAttach() override {}
		void OnDetach() override {}
		void OnEvent(Event& event) override {}
		void OnUpdate() override;
	};
}
