#ifndef _GALAXYSAILING_VULKAN_PASS_H_
#define _GALAXYSAILING_VULKAN_PASS_H_

#include "vulkan_context.h"
#include "vulkan_buffer.h"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace galaxysailing{
    
    class VulkanTestPass{
    public:
        struct TestUniform{
            alignas(16) glm::mat4 proj_view_model;
            alignas(16) glm::mat3 trans_inv_model;
        };
        void initialize(VulkanContext* context, VkDescriptorPool pool, int image_count){
            m_vkContext = context;
            m_descriptorPool = pool;
            m_uniformBuffers.resize(image_count);
            TestUniform uniform;
            for (int i = 0; i < m_uniformBuffers.size(); ++i)
            {
                m_uniformBuffers[i] = std::make_shared<VulkanBuffer>(m_vkContext, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                m_uniformBuffers[i]->create<TestUniform>(&uniform, 1);
            }
            _createDescriptorSetLayout();
            _createDescriptorSets(image_count);
            _createRenderPass();
            _createPipeline();
            _createFramebuffers();
        }
        void draw(VkBuffer vertex_buffer, VkBuffer index_buffer, int cnt, VkCommandBuffer cmd_buffer, int image_index);
        void updateUniform(TestUniform uniform, int image_index);
        void recreateFramebuffers();
    private:
        void _createDescriptorSetLayout();
        void _createDescriptorSets(int image_count);
        void _createRenderPass();
        void _createPipeline();
        void _createFramebuffers();
    private:
        VkPipeline m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        VkRenderPass m_renderPass;
        std::vector<std::shared_ptr<VulkanBuffer>> m_uniformBuffers;

        VulkanContext* m_vkContext;
        VkDescriptorPool m_descriptorPool;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::vector<VkDescriptorSet> m_descriptorSets;
        
        std::vector<VkFramebuffer> m_swapchainFramebuffers;
    };

}

#endif