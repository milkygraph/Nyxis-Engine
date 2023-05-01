#pragma once
#include <functional>
#include "Scene/Scene.hpp"

namespace Nyxis
{
	class ComponentViewPanel
	{
	public:
		ComponentViewPanel() = default;
		~ComponentViewPanel() = default;
		void OnUpdate();
		template<typename T>
		void DrawComponentNode(const char* name, Entity entity, std::function<void(T& component)> func);
	};
}
