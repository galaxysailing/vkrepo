#ifndef _GALAXYSAILING_VULKAN_MANAGER_H_
#define _GALAXYSAILING_VULKAN_MANAGER_H_

#include "vulkan_context.h"
#include "vulkan_buffer.h"
#include "vulkan_pass.h"
#include <vector>
#include <map>
#include <memory>
namespace galaxysailing{
    class VulkanManager{
    public:
        void initialize(GLFWwindow* window);
        void renderFrame(int render_id);
        int addMesh(std::vector<uint32_t> indices, std::vector<float> vertices);
    private:
        struct VulkanMeshBuffer{
            std::shared_ptr<VulkanBuffer> index_buffer;
            std::shared_ptr<VulkanBuffer> vertex_buffer;
        };

        // vulkan device context
        VulkanContext m_vkContext;
        uint32_t const m_maxFramesInFlight = 3;
        uint32_t m_currentFrameIndex = 0;

        // sync object
        std::vector<VkFence> m_inFlightFences;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;

        // command
        VkCommandPool m_commandPool;
        std::vector<VkCommandBuffer> m_commandBuffers;

        // Pass
        VulkanTestPass m_testPass;

        VkDescriptorPool m_descriptorPool;
        
        std::map<uint32_t, VulkanMeshBuffer> m_vulkanMeshBufferMap;
    private:
        void createCommandPool();
        void createCommandBuffers();
        void createDescriptorPool();
        void createSyncObjects();
        void recreateSwapchain();
    };
}

#endif