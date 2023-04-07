#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Nyxis.hpp"
#include "Scene/Scene.hpp"

namespace Nyxis
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		~SceneHierarchyPanel() = default;
		void OnUpdate();
		void DrawEntityNode(Entity entity) const;
	};

}
