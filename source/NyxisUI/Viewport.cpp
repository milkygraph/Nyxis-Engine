#include "NyxisUI/Viewport.hpp"
#include "Core/Application.hpp"
#include "Core/Renderer.hpp"
#include "Events/MouseEvents.hpp"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <ImGuizmo/ImGuizmo.h>

namespace Nyxis
{
	// snap values
	static const float TRANSLATE_SNAP_VALUES[3] = { 0.1f, 0.1f, 0.1f };
	static const float ROTATE_SNAP_VALUES[3] = { 45.0f, 45.0f, 45.0f };
	static const float SCALE_SNAP_VALUES[3] = { 0.1f, 0.1f, 0.1f };

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		if(glm::epsilonEqual(transform[3][3], 0.0f, glm::epsilon<float>()))
			return false;

		glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));

		// Convert right-handed matrix to left-handed
		glm::mat4 leftTransform = flipY * transform * flipY;

		// Extract position, rotation, and scale vectors
		translation = glm::vec3(leftTransform[3]);
		scale.x = glm::length(glm::vec3(leftTransform[0]));
		scale.y = glm::length(glm::vec3(leftTransform[1]));
		scale.z = glm::length(glm::vec3(leftTransform[2]));

		glm::mat3 rotationMatrix;
		rotationMatrix[0] = glm::vec3(leftTransform[0]) / scale.x;
		rotationMatrix[1] = glm::vec3(leftTransform[1]) / scale.y;
		rotationMatrix[2] = glm::vec3(leftTransform[2]) / scale.z;

		auto rotationQuat = glm::quat_cast(rotationMatrix);
		rotation = glm::eulerAngles(rotationQuat);

		return true;
	}

	Viewport::Viewport()
	{
		m_DescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (auto& descriptorSet : m_DescriptorSets)
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

		if (vkCreateSampler(Device::Get().device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create viewport sampler!");
		}
	}

	Viewport::~Viewport()
	{
		vkDestroySampler(Device::Get().device(), m_Sampler, nullptr);
	}

	void Viewport::UpdateGizmo()
	{
		// TODO: Fix handling of gizmo with y and z axis
		auto frameInfo = Application::GetFrameInfo();
		glm::vec2 mousePos = Input::GetMousePosition();

		auto selected_entity = EditorLayer::GetSelectedEntity();
		if(selected_entity != entt::null)
		{
			auto scene = Application::GetScene();

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::AllowAxisFlip(true);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_WindowPos.x, m_WindowPos.y, m_WindowSize.x, m_WindowSize.y);

			auto camera = scene->GetCamera();
			auto view = camera->getViewMatrix();
			auto projection = camera->getProjectionMatrix();
			projection[1][1] *= -1.0f;

			auto& transform = scene->GetComponent<TransformComponent>(selected_entity);
			auto modelMatrix = transform.mat4(true);

			if (m_DrawGizmos)
			{
				const float snapValues[3] = { m_SnapValue, m_SnapValue, m_SnapValue };
				ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
				                     m_CurrentGizmoOperation, m_CurrentGizmoMode, glm::value_ptr(modelMatrix), nullptr, m_GizmoSnapping ? snapValues: nullptr);
			}
				
			m_OverGizmo = ImGuizmo::IsOver();
			m_UsingGizmo = ImGuizmo::IsUsing();

			if (m_UsingGizmo)
			{
				glm::vec3 translation, rotation, scale;
				DecomposeTransform(glm::mat4(modelMatrix), translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - transform.rotation;
				transform.translation = translation;
				transform.rotation += deltaRotation;
				transform.scale = scale;
				mousePos = { -1, -1 };
			}
		}

		mousePos.x -= m_WindowPos.x;
		mousePos.y -= m_WindowPos.y;

		if (mousePos.x < 0 || mousePos.x > m_WindowSize.y)
			mousePos.x = -1;
		if (mousePos.y < 0 || mousePos.y > m_WindowSize.x)
			mousePos.y = -1;

		frameInfo->mousePosition = mousePos;
	}

	void Viewport::OnUpdate()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		// ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0)); // Set the alpha channel to 0.5
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
		             ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
	
		m_WindowPos = ImGui::GetWindowPos();
		m_WindowSize = ImGui::GetWindowSize();
		m_IsFocused = ImGui::IsWindowFocused();
		m_IsHovered = ImGui::IsWindowHovered();
	
		UpdateViewport();
		UpdateGizmo();

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Viewport::OnEvent(Event& event)
	{
		// do not process any events if the window is not focused
		if (!m_IsHovered)
			return;

		switch(event.getEventType())
		{
		case(EventType::KeyPressed):
			if(Input::IsKeyPressed(T))
				m_CurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			if(Input::IsKeyPressed(R))
				m_CurrentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
 			if(Input::IsKeyPressed(S))
				m_CurrentGizmoOperation = ImGuizmo::OPERATION::SCALE;
			if(Input::IsKeyPressed(W))
				m_CurrentGizmoMode = ImGuizmo::MODE::WORLD;
			if(Input::IsKeyPressed(L))
				m_CurrentGizmoMode = ImGuizmo::MODE::LOCAL;
			if(Input::IsKeyPressed(LeftControl) && Input::IsKeyPressed(D))
				EditorLayer::DeselectEntity();
			break;
		case(EventType::MouseButtonPressed):
			if(Input::IsMouseButtonPressed(MouseButtonLeft))
			{
				if (Input::IsKeyPressed(LeftControl))
				{
					m_GizmoSnapping = true;
					if (m_CurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE || m_CurrentGizmoOperation == ImGuizmo::OPERATION::SCALE)
					{
						m_SnapValue = 0.1f;
					}
					else
					{
						m_SnapValue = 15.0f;
					}
				}
				else
					m_GizmoSnapping = false;
				break;
			}
			if(Input::IsMouseButtonPressed(MouseButtonRight))
			{
				Input::SetCursorMode(CursorDisabled);
				Application::GetScene()->SetCameraControl(true);
			}


			if(Input::IsMouseButtonReleased(MouseButtonRight))
			{
				Input::SetCursorMode(CursorNormal);
				Application::GetScene()->SetCameraControl(false);
			}
		}

		if(event.getEventType() == EventType::MouseButtonPressed)
		{
			if(Input::IsMouseButtonPressed(MouseButtonLeft))
			{
				Input::SetCursorMode(CursorDisabled);
				Application::GetScene()->SetCameraControl(true);
			}
		}
	}

	void Viewport::UpdateViewport()
	{
		const auto frameInfo = Application::GetFrameInfo();
		const auto imageView = Renderer::GetWorldImageView(frameInfo->frameIndex);

		if (m_DescriptorSets[frameInfo->frameIndex] == VK_NULL_HANDLE)
			m_DescriptorSets[frameInfo->frameIndex] = ImGui_ImplVulkan_AddTexture(
				m_Sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else
		{
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler = m_Sampler;
			desc_image[0].imageView = imageView;
			desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = m_DescriptorSets[frameInfo->frameIndex];
			write_desc[0].descriptorCount = 1;
			write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc[0].pImageInfo = desc_image;
			vkUpdateDescriptorSets(Device::Get().device(), 1, write_desc, 0, nullptr);
		}

		// display image in the middle of the window with the correct aspect ratio
		ImGui::Image(m_DescriptorSets[frameInfo->frameIndex], m_WindowSize);
	}
}
