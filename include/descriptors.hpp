#pragma once

#include "device.hpp"

#include "Nyxispch.hpp"

namespace Nyxis
{

    class veDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder() {}

            Builder &addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<veDescriptorSetLayout> build() const;

        private:
          Device &device = Device::get();
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        veDescriptorSetLayout(
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~veDescriptorSetLayout();
        veDescriptorSetLayout(const veDescriptorSetLayout &) = delete;
        veDescriptorSetLayout &operator=(const veDescriptorSetLayout &) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
      Device &device = Device::get();
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class veDescriptorWriter;
    };

    class veDescriptorPool
    {
    public:
        VkDescriptorPool getDescriptorPool() {return descriptorPool;}
        class Builder
        {
        public:
            Builder() {}

            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<veDescriptorPool> build() const;

        private:
          Device &device = Device::get();
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        veDescriptorPool(
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~veDescriptorPool();
        veDescriptorPool(const veDescriptorPool &) = delete;
        veDescriptorPool &operator=(const veDescriptorPool &) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void resetPool();

    private:
      Device &device = Device::get();
        VkDescriptorPool descriptorPool;

        friend class veDescriptorWriter;
    };

    class veDescriptorWriter
    {
    public:
        veDescriptorWriter(veDescriptorSetLayout &setLayout, veDescriptorPool &pool);

        veDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        veDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        veDescriptorSetLayout &setLayout;
        veDescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

} // namespace Nyxis
