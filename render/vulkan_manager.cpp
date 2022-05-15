#include "vulkan_manager.h"
#include "vulkan_util.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace galaxysailing
{
    void VulkanManager::initialize(GLFWwindow* window)
    {
        m_vkContext.initialize(window);

        createCommandPool();
        createCommandBuffers();
        createDescriptorPool();
        createSyncObjects();

        m_testPass.initialize(&m_vkContext, m_descriptorPool, m_vkContext.m_swapchainImages.size());
    }

    void VulkanManager::renderFrame(int render_id)
    {
        VK_CHECK_RESULT(m_vkContext.fp_vkWaitForFences(m_vkContext.m_device, 1, &m_inFlightFences[m_currentFrameIndex], VK_TRUE, UINT64_MAX));
        uint32_t image_index;
        VkResult result = vkAcquireNextImageKHR(m_vkContext.m_device
            , m_vkContext.m_swapchain
            , UINT64_MAX
            , m_imageAvailableSemaphores[m_currentFrameIndex]
            , VK_NULL_HANDLE
            , &image_index);

        // std::cout<<"image_index: " << image_index << " currentFrameIndex: " << m_currentFrameIndex << "\n"; 

        if(result == VK_ERROR_OUT_OF_DATE_KHR){
            // Swapchain has become incompatible with the surface.
            // Usually happens after a window resize.
            recreateSwapchain();
            return;
        }else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // begin command buffer
        VkCommandBufferBeginInfo cmd_buffer_begin_info {};
        cmd_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_buffer_begin_info.flags            = 0;
        cmd_buffer_begin_info.pInheritanceInfo = nullptr;
        VK_CHECK_RESULT(m_vkContext.fp_vkBeginCommandBuffer(m_commandBuffers[m_currentFrameIndex], &cmd_buffer_begin_info));

        // TODO Draw
        static float angle = 0;
        VulkanTestPass::TestUniform uniform;
        glm::mat4 model(1.0f), view(1.0f), projection(1.0);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        angle += 0.01;
        model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
        glm::vec3 eyepos(0.0f, 0.0f, 5.0f);
        glm::vec3 front(0.0f, 0.0f, -1.0f);
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        view = glm::lookAt(eyepos, eyepos + front, up);
        float aspect = (float)m_vkContext.m_swapchainExtent.width / m_vkContext.m_swapchainExtent.height;
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
        projection[1][1] *= -1;
        uniform.proj_view_model = projection * view * model;
        uniform.trans_inv_model = glm::transpose(glm::inverse(glm::mat3(model)));
        m_testPass.updateUniform(uniform, image_index);
        auto& mesh_buffer = m_vulkanMeshBufferMap[render_id];
        int size = mesh_buffer.index_buffer->getSize();
        m_testPass.draw(mesh_buffer.vertex_buffer->getBuffer()
            , mesh_buffer.index_buffer->getBuffer()
            , size
            , m_commandBuffers[m_currentFrameIndex]
            , image_index);

        // ----- ----- ----- ----- ----- ----- ----- ----- ----- -----

        // end command buffer
        VK_CHECK_RESULT(m_vkContext.fp_vkEndCommandBuffer(m_commandBuffers[m_currentFrameIndex]));  

        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &m_imageAvailableSemaphores[m_currentFrameIndex];
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffers[m_currentFrameIndex];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &m_renderFinishedSemaphores[m_currentFrameIndex];

        // reset fence
        m_vkContext.fp_vkResetFences(m_vkContext.m_device, 1, &m_inFlightFences[m_currentFrameIndex]);
        VK_CHECK_RESULT(vkQueueSubmit(m_vkContext.m_graphicsQueue
            , 1
            , &submit_info
            , m_inFlightFences[m_currentFrameIndex]));
        
        // present swapchain
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrameIndex];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_vkContext.m_swapchain;
        presentInfo.pImageIndices = &image_index;
        // It's not necessary if you're only using a single swap chain
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(m_vkContext.m_presentQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            recreateSwapchain();
        }
        else if (result != VK_SUCCESS) {
            VK_CHECK_RESULT(result);
        }

        m_currentFrameIndex = (m_currentFrameIndex + 1) % m_maxFramesInFlight;
    }

    int VulkanManager::addMesh(std::vector<uint32_t> indices, std::vector<float> vertices){
        static int global_id = 0;

        VulkanMeshBuffer mesh_buffer;
        mesh_buffer.index_buffer = std::make_shared<VulkanBuffer>(&m_vkContext
            , VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        mesh_buffer.index_buffer->create<uint32_t>(indices.data(), indices.size());

        mesh_buffer.vertex_buffer = std::make_shared<VulkanBuffer>(&m_vkContext
            , VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        mesh_buffer.vertex_buffer->create<float>(vertices.data(), vertices.size());

        m_vulkanMeshBufferMap.emplace(global_id, mesh_buffer);
        return global_id++;
    }

// ----------------------- initialize -------------------------

    void VulkanManager::createCommandPool(){
        VkCommandPoolCreateInfo pool_create_info{};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.queueFamilyIndex = m_vkContext.m_queueFamilyIndices.graphicsFamily.value();
        pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VK_CHECK_RESULT(vkCreateCommandPool(m_vkContext.m_device, &pool_create_info, nullptr, &m_commandPool));
    }

    void VulkanManager::createCommandBuffers(){
        m_commandBuffers.resize(m_maxFramesInFlight);
        VkCommandBufferAllocateInfo cmd_buffer_allocate_info{};
        cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_buffer_allocate_info.commandPool = m_commandPool;
        cmd_buffer_allocate_info.commandBufferCount = 1U;

        for(size_t i = 0; i < m_maxFramesInFlight; ++i){
            VK_CHECK_RESULT(vkAllocateCommandBuffers(m_vkContext.m_device, &cmd_buffer_allocate_info, &m_commandBuffers[i]));
        }
    }

    void VulkanManager::createDescriptorPool(){
        auto& ctx = m_vkContext;
        std::array<VkDescriptorPoolSize, 1> descpool_size{};
        descpool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descpool_size[0].descriptorCount = 1 * static_cast<uint32_t>(ctx.m_swapchainImages.size());

        VkDescriptorPoolCreateInfo descpool_ci{};
        descpool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descpool_ci.poolSizeCount = descpool_size.size();
        descpool_ci.pPoolSizes = descpool_size.data();
        descpool_ci.maxSets = static_cast<uint32_t>(ctx.m_swapchainImages.size());

        if (vkCreateDescriptorPool(ctx.m_device, &descpool_ci, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void VulkanManager::createSyncObjects(){
        // semaphonre for GPU-GPU synchronized
        // fence for CPU-GPU synchronized
        m_imageAvailableSemaphores.resize(m_maxFramesInFlight);
        m_renderFinishedSemaphores.resize(m_maxFramesInFlight);
        m_inFlightFences.resize(m_maxFramesInFlight);
        // imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // default: UNSIGNALED
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < m_maxFramesInFlight; i++)
        {
            if (vkCreateSemaphore(m_vkContext.m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_vkContext.m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_vkContext.m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- 

// ----------------------- swapchain --------------------------
    void VulkanManager::recreateSwapchain()
    {
    // ------------------- Handling minimization -----------------------
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_vkContext.m_window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_vkContext.m_window, &width, &height);
            glfwWaitEvents();
        }
    // ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- 

        // vkDeviceWaitIdle(m_vkContext.m_device);
        VK_CHECK_RESULT(m_vkContext.fp_vkWaitForFences(m_vkContext.m_device
            , m_maxFramesInFlight
            , m_inFlightFences.data()
            , VK_TRUE
            , UINT64_MAX));

        m_vkContext.clearSwapchain();
        
        m_vkContext.createSwapchain();
        m_vkContext.createSwapchainImageViews();
        m_vkContext.createDepthResources();

        // todo recreate other framebuffer
        m_testPass.recreateFramebuffers();
    }

// ----- ----- ----- ----- ----- ----- ----- ----- ----- -----

}