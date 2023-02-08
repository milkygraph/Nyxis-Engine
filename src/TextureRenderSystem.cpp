#include "TextureRenderSystem.hpp"
#include "frameInfo.hpp"

namespace ve
{
	struct TexturePushConstantData {
		glm::mat4 modelMatrix{1.f};
		glm::mat4 normalMatrix{1.f};
	};

	TextureRenderSystem::TextureRenderSystem(VkRenderPass RenderPass, VkDescriptorSetLayout globalDescriptorSetLayout)
	{
		CreatePipelineLayout(globalDescriptorSetLayout);
		CreatePipeline(RenderPass);
	}

	TextureRenderSystem::~TextureRenderSystem()
	{
		vkDestroyPipelineLayout(pDevice.device(), pipelineLayout, nullptr);
	}

	void TextureRenderSystem::OnUpdate()
	{

	}

	void TextureRenderSystem::Render(FrameInfo& frameInfo)
	{
		pPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);

		frameInfo.scene.m_Registry.view<TransformComponent, MeshComponent, Texture>().each([&](auto entity, auto& transform, auto& mesh, auto& texture)
		{
		  auto& model = *mesh.model;
			  if(model.loaded)
			  {
				  auto imageInfo = texture.GetDescriptorImageInfo();
				  VkDescriptorSet descriptorSet;
				  veDescriptorWriter(*TextureSetLayout, frameInfo.TexturePool)
				        .writeImage(0, &imageInfo)
					    .build(descriptorSet);

				  vkCmdBindDescriptorSets(
					  frameInfo.commandBuffer,
					  VK_PIPELINE_BIND_POINT_GRAPHICS,
					  pipelineLayout,
					  1,  // first set
					  1,  // set count
					  &descriptorSet,
					  0,
					  nullptr);

				  TexturePushConstantData push{};
				  push.modelMatrix = transform.mat4 ();
				  push.normalMatrix = transform.normalMatrix ();

				  vkCmdPushConstants (
					  frameInfo.commandBuffer,
					  pipelineLayout,
					  VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				  0,
				  sizeof (TexturePushConstantData),
				  &push);

			  model.bind (frameInfo.commandBuffer);
			  model.draw (frameInfo.commandBuffer);
		  }
		});

	}

	void TextureRenderSystem::Cleanup()
	{

	}

	void TextureRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(TexturePushConstantData);

		TextureSetLayout =
			veDescriptorSetLayout::Builder()
				.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			globalDescriptorSetLayout,
			TextureSetLayout->getDescriptorSetLayout()};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(pDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void TextureRenderSystem::CreatePipeline(VkRenderPass renderPass)
	{
		PipelineConfigInfo pipelineConfig{};
		vePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		pPipeline = std::make_unique<vePipeline>(
			"../shaders/texture_shader.vert.spv",
			"../shaders/texture_shader.frag.spv",
			pipelineConfig);
	}
}
