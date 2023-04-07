#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Nyxis.hpp"

namespace Nyxis
{
	class ComponentViewPanel
	{
	public:
		ComponentViewPanel() = default;
		~ComponentViewPanel() = default;
		void OnUpdate();
	};
}