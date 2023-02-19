#include "descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace Nyxis
{

  // *************** Descriptor Set Layout Builder *********************

  veDescriptorSetLayout::Builder &veDescriptorSetLayout::Builder::addBinding(
      uint32_t binding,
      VkDescriptorType descriptorType,
      VkShaderStageFlags stageFlags,
      uint32_t count)
  {
    assert(bindings.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
  }

  std::unique_ptr<veDescriptorSetLayout> veDescriptorSetLayout::Builder::build() const
  {
    return std::make_unique<veDescriptorSetLayout>(bindings);
  }

  // *************** Descriptor Set Layout *********************

  veDescriptorSetLayout::veDescriptorSetLayout(
      std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
      : bindings{bindings}
  {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv : bindings)
    {
      setLayoutBindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(
            device.device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptorSetLayout) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create descriptor set layout!");
    }
  }

  veDescriptorSetLayout::~veDescriptorSetLayout()
  {
    vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
  }

  // *************** Descriptor Pool Builder *********************

  veDescriptorPool::Builder &veDescriptorPool::Builder::addPoolSize(
      VkDescriptorType descriptorType, uint32_t count)
  {
    poolSizes.push_back({descriptorType, count});
    return *this;
  }

  veDescriptorPool::Builder &veDescriptorPool::Builder::setPoolFlags(
      VkDescriptorPoolCreateFlags flags)
  {
    poolFlags = flags;
    return *this;
  }
  veDescriptorPool::Builder &veDescriptorPool::Builder::setMaxSets(uint32_t count)
  {
    maxSets = count;
    return *this;
  }

  std::unique_ptr<veDescriptorPool> veDescriptorPool::Builder::build() const
  {
    return std::make_unique<veDescriptorPool>(maxSets, poolFlags, poolSizes);
  }

  // *************** Descriptor Pool *********************

  veDescriptorPool::veDescriptorPool(
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes)
  {
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
        VK_SUCCESS)
    {
      throw std::runtime_error("failed to create descriptor pool!");
    }
  }

  veDescriptorPool::~veDescriptorPool()
  {
    vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
  }

  bool veDescriptorPool::allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const
  {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptor) != VK_SUCCESS)
    {
      return false;
    }
    return true;
  }

  void veDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const
  {
    vkFreeDescriptorSets(
        device.device(),
        descriptorPool,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data());
  }

  void veDescriptorPool::resetPool()
  {
    vkResetDescriptorPool(device.device(), descriptorPool, 0);
  }

  // *************** Descriptor Writer *********************

  veDescriptorWriter::veDescriptorWriter(veDescriptorSetLayout &setLayout, veDescriptorPool &pool)
      : setLayout{setLayout}, pool{pool} {}

  veDescriptorWriter &veDescriptorWriter::writeBuffer(
      uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
  {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = setLayout.bindings[binding];

    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    writes.push_back(write);
    return *this;
  }

  veDescriptorWriter &veDescriptorWriter::writeImage(
      uint32_t binding, VkDescriptorImageInfo *imageInfo)
  {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = setLayout.bindings[binding];

    assert(
        bindingDescription.descriptorCount == 1 &&
        "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    writes.push_back(write);
    return *this;
  }

  bool veDescriptorWriter::build(VkDescriptorSet &set)
  {
    bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
    if (!success)
    {
      return false;
    }
    overwrite(set);
    return true;
  }

  void veDescriptorWriter::overwrite(VkDescriptorSet &set)
  {
    for (auto &write : writes)
    {
      write.dstSet = set;
    }
    vkUpdateDescriptorSets(pool.device.device(), writes.size(), writes.data(), 0, nullptr);
  }

} // namespace Nyxis
