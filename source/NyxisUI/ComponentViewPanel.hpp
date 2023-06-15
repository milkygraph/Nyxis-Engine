#pragma once
#include <functional>

#include "Core/Layer.hpp"
#include "Scene/Scene.hpp"

namespace Nyxis
{
	class ComponentViewPanel final : public Layer
	{
	public:
		ComponentViewPanel() = default;
		~ComponentViewPanel() override = default;
		void OnAttach() override {}
		void OnDetach() override {}
		void OnEvent(Event& event) override {}
		void OnUpdate() override;

	private:
		template<typename T>
		void DrawComponentNode(const char* name, Entity entity, std::function<void(T& component)> func);
	};
}
