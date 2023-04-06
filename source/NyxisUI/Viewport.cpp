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

		float snapValue = 0.1f;
		// draw gizmos
		if(!Input::isMouseButtonPressed(MouseCodes::MouseButtonRight))
		{
 			if(Input::isKeyPressed(KeyCodes::S))
				m_CurrentGizmoOperation = ImGuizmo::OPERATION::SCALE;
			if(Input::isKeyPressed(KeyCodes::R))
				m_CurrentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
			if(Input::isKeyPressed(KeyCodes::T))
				m_CurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			if(Input::isKeyPressed(KeyCodes::W))
				m_CurrentGizmoMode = ImGuizmo::MODE::WORLD;
			if(Input::isKeyPressed(KeyCodes::L))
				m_CurrentGizmoMode = ImGuizmo::MODE::LOCAL;
			if(Input::isKeyPressed(KeyCodes::LeftControl))
			{
				m_Snap = true;
				if (m_CurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE || m_CurrentGizmoOperation == ImGuizmo::OPERATION::SCALE)
				{
					snapValue = 0.1f;
				}
				else
				{
					snapValue = 15.0f;
				}
			}
			else
				m_Snap = false;
		}

		auto selected_entity = EditorLayer::GetSelectedEntity();
		if(selected_entity != entt::null)
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

			auto& rigidBodyEntity = scene->GetComponent<RigidBody>(selected_entity);
			auto modelMatrix = rigidBodyEntity.mat4(true);

			if (m_DrawGizmos)
			{
				const float snapValues[3] = { snapValue, snapValue, snapValue };
				ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
					m_CurrentGizmoOperation, m_CurrentGizmoMode, glm::value_ptr(modelMatrix), nullptr, m_Snap ? snapValues: nullptr);
			}
				
			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				DecomposeTransform(glm::mat4(modelMatrix), translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - rigidBodyEntity.rotation;
				rigidBodyEntity.translation = translation;
				rigidBodyEntity.rotation += deltaRotation;
				rigidBodyEntity.scale = scale;
				mousePos = { -1, -1 };
			}
		}

		m_IsFocused = ImGui::IsWindowFocused();

		ImGui::End();
		ImGui::PopStyleVar();

		mousePos.x -= windowPos.x;
		mousePos.y -= windowPos.y;

		if (mousePos.x < 0 || mousePos.x > windowWidth)
			mousePos.x = -1;
		if (mousePos.y < 0 || mousePos.y > windowHeight)
			mousePos.y = -1;

		frameInfo->mousePosition = mousePos;
	}

	void Viewport::OnEvent()
	{
		// EventDispatcher dispatcher(e);
		// dispatcher.Dispatch<MouseButtonReleasedEvent>(NYX_BIND_EVENT_FN(Viewport::OnMouseButtonReleased));
	}
}
