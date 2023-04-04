#include "NyxisUI/Viewport.hpp"
#include "Core/Application.hpp"
#include "Core/Renderer.hpp"
#include "Events/MouseEvents.hpp"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <ImGuizmo/ImGuizmo.h>

namespace Nyxis
{
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		if(glm::epsilonEqual(transform[3][3], 0.0f, glm::epsilon<float>()))
			return false;

		glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));

		// Convert right-handed matrix to left-handed
		glm::mat4 leftTransform = flipY * transform * flipY;

		// Extract position, rotation, and scale vectors
		translation = glm::vec3(leftTransform[3]);
		rotation = glm::degrees(glm::eulerAngles(glm::toQuat(leftTransform)));
		scale.x = glm::length(glm::vec3(leftTransform[0]));
		scale.y = glm::length(glm::vec3(leftTransform[1]));
		scale.z = glm::length(glm::vec3(leftTransform[2]));

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

	void Viewport::OnUpdate()
	{
		auto frameInfo = Application::GetFrameInfo();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		// ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0)); // Set the alpha channel to 0.5
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
		             ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

		auto imageView = Renderer::GetWorldImageView(frameInfo->frameIndex);

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
		float windowWidth = ImGui::GetWindowWidth();
		float windowHeight = ImGui::GetWindowHeight();

		// display image in the middle of the window with the correct aspect ratio
		ImGui::Image(m_DescriptorSets[frameInfo->frameIndex], ImVec2(windowWidth, windowHeight));

		auto windowPos = ImGui::GetWindowPos();
		// check if mouse is in window
		if (ImGui::IsWindowHovered() && Input::isMouseButtonPressed(MouseCodes::MouseButtonRight))
		{
			Input::setCursorMode(CursorMode::CursorDisabled);
			Application::GetScene()->SetCameraControl(true);
		}

		m_Extent = { static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight) };
		// get mouse position relative to the window
		glm::vec2 mousePos = Input::getMousePosition();

		mousePos.x -= windowPos.x;
		mousePos.y -= windowPos.y;

		if (mousePos.x < 0 || mousePos.x > windowWidth)
			mousePos.x = -1;
		if (mousePos.y < 0 || mousePos.y > windowHeight)
			mousePos.y = -1;

		// draw gizmos
		if (static_cast<uint32_t>(EditorLayer::GetSelectedEntity()) != entt::null)
		{
			auto scene = Application::GetScene();

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::AllowAxisFlip(true);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(windowPos.x, windowPos.y, windowWidth, windowHeight);

			auto camera = scene->GetCamera();
			auto view = camera->getViewMatrix();
			auto projection = camera->getProjectionMatrix();
			projection[1][1] *= -1.0f;

			auto selected_entity = EditorLayer::GetSelectedEntity();
			auto& rigidBodyEntity = scene->GetComponent<RigidBody>(selected_entity);

			auto modelMatrix = rigidBodyEntity.mat4(true);

			ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
				m_CurrentGizmoOperation, m_CurrentGizmoMode, glm::value_ptr(modelMatrix));

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				DecomposeTransform(glm::mat4(modelMatrix), translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - rigidBodyEntity.rotation;
				rigidBodyEntity.translation = translation;
				rigidBodyEntity.rotation += deltaRotation;
				rigidBodyEntity.scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("control");
		// controls for gizmo
		if (ImGui::RadioButton("Translate", m_CurrentGizmoOperation == ImGuizmo::TRANSLATE))
			m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", m_CurrentGizmoOperation == ImGuizmo::ROTATE))
			m_CurrentGizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", m_CurrentGizmoOperation == ImGuizmo::SCALE))
			m_CurrentGizmoOperation = ImGuizmo::SCALE;
		if (ImGui::RadioButton("World", m_CurrentGizmoMode == ImGuizmo::WORLD))
			m_CurrentGizmoMode = ImGuizmo::WORLD;
		ImGui::SameLine();
		if (ImGui::RadioButton("Local", m_CurrentGizmoMode == ImGuizmo::LOCAL))
			m_CurrentGizmoMode = ImGuizmo::LOCAL;

		if (m_CurrentGizmoOperation != m_LastGizmoOperation)
		{
			m_CurrentGizmoMode = ImGuizmo::LOCAL;
			m_LastGizmoOperation = m_CurrentGizmoOperation;
		}
		ImGui::End();

		frameInfo->mousePosition = mousePos;
	}

	void Viewport::OnEvent()
	{
		// EventDispatcher dispatcher(e);
		// dispatcher.Dispatch<MouseButtonReleasedEvent>(NYX_BIND_EVENT_FN(Viewport::OnMouseButtonReleased));
	}
}
