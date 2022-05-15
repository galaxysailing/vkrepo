#include "vulkan_pass.h"
#include "vulkan_util.h"
#include <array>
#include "vulkan_mesh.h"
namespace galaxysailing {
    void VulkanTestPass::draw(VkBuffer vertex_buffer, VkBuffer index_buffer, int cnt, VkCommandBuffer cmd_buffer, int image_index){
        auto& ctx = *m_vkContext;
        {
            VkRenderPassBeginInfo renderpass_begin_info {};
            renderpass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpass_begin_info.renderPass        = m_renderPass;
            renderpass_begin_info.framebuffer       = m_swapchainFramebuffers[image_index];
            renderpass_begin_info.renderArea.offset = {0, 0};
            renderpass_begin_info.renderArea.extent = ctx.m_swapchainExtent;

            std::array<VkClearValue, 2> clear_values;
            clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clear_values[1].depthStencil = { 1.0f, 0 };
            renderpass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
            renderpass_begin_info.pClearValues = clear_values.data();
            ctx.fp_vkCmdBeginRenderPass(cmd_buffer, &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        }
        ctx.fp_vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)ctx.m_swapchainExtent.width;
        viewport.height = (float)ctx.m_swapchainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = ctx.m_swapchainExtent;

        ctx.fp_vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
        ctx.fp_vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
        VkBuffer vertex_Buffers[] = { vertex_buffer };
        VkDeviceSize offsets[] = { 0 };
        ctx.fp_vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertex_Buffers, offsets);
        ctx.fp_vkCmdBindIndexBuffer(cmd_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
        ctx.fp_vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[image_index], 0, nullptr);
        
        ctx.fp_vkCmdDrawIndexed(cmd_buffer, static_cast<int>(cnt), 1, 0, 0, 0);
        ctx.fp_vkCmdEndRenderPass(cmd_buffer);
    }

    void VulkanTestPass::updateUniform(TestUniform uniform, int image_index){
        
        m_uniformBuffers[image_index]->update<TestUniform>(&uniform);
    }

    void VulkanTestPass::recreateFramebuffers(){
        for(int i = 0; i < m_swapchainFramebuffers.size(); ++i){
            vkDestroyFramebuffer(m_vkContext->m_device, m_swapchainFramebuffers[i], nullptr);
        }
        _createFramebuffers();
    }

// private:
    void VulkanTestPass::_createDescriptorSetLayout(){
        auto& ctx = *m_vkContext;
        VkDescriptorSetLayoutBinding ubo_layout_binding{};
        ubo_layout_binding.binding = 0;
        ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.descriptorCount = 1;
        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        // only relevant for image sampling related descriptors
        ubo_layout_binding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = 1U;
        layout_info.pBindings = &ubo_layout_binding;

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(ctx.m_device, &layout_info, nullptr, &m_descriptorSetLayout));
    }

    void VulkanTestPass::_createDescriptorSets(int image_count){
        auto& ctx = *m_vkContext;
        std::vector<VkDescriptorSetLayout> layouts(image_count, m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = m_descriptorPool;
        alloc_info.descriptorSetCount = static_cast<uint32_t>(image_count);
        alloc_info.pSetLayouts = layouts.data();

        m_descriptorSets.resize(image_count);
        VK_CHECK_RESULT(vkAllocateDescriptorSets(ctx.m_device, &alloc_info, m_descriptorSets.data()));

        for(size_t i = 0; i < image_count; ++i){
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = m_uniformBuffers[i]->getBuffer();
            buffer_info.offset = 0;
            buffer_info.range = sizeof(TestUniform);

            VkWriteDescriptorSet write_desc_set{};
            write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_desc_set.dstSet = m_descriptorSets[i];
            write_desc_set.dstBinding = 0;
            write_desc_set.dstArrayElement = 0;
            write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_desc_set.descriptorCount = 1;
            write_desc_set.pBufferInfo = &buffer_info;

            vkUpdateDescriptorSets(ctx.m_device, 1, &write_desc_set, 0, nullptr);
        }
    }

    void VulkanTestPass::_createRenderPass(){
        VulkanContext& ctx = *m_vkContext;

        // 1. attach ref
        // layout(location = 0) out vec4 FragColor
        VkAttachmentReference color_attach_ref{};
        color_attach_ref.attachment = 0;
        color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depth_attach_ref{};
        depth_attach_ref.attachment = 1;
        depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // 2. subpass
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attach_ref;
        subpass.pDepthStencilAttachment = &depth_attach_ref;

        // 3. subpass dependency
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        // 4. attach desc
        // color attach
        VkAttachmentDescription color_attach{};
        color_attach.format = ctx.m_swapchainImageFormat;
        color_attach.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        // depth attach
        VkAttachmentDescription depth_attach{};
        depth_attach.format = ctx.m_depthImageFormat;
        depth_attach.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // 5. create renderpass
        std::array<VkAttachmentDescription, 2> attachments = { 
            color_attach, 
            depth_attach
        };
        VkRenderPassCreateInfo render_pass_ci{};
        render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_ci.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_ci.pAttachments = attachments.data();
        render_pass_ci.subpassCount = 1;
        render_pass_ci.pSubpasses = &subpass;
        render_pass_ci.dependencyCount = 1;
        render_pass_ci.pDependencies = &dependency;

        VK_CHECK_RESULT(vkCreateRenderPass(ctx.m_device, &render_pass_ci, nullptr, &m_renderPass));
        
    }

    void VulkanTestPass::_createPipeline(){
        auto& ctx = *m_vkContext;
        auto vert_code = VulkanUtil::compileFile("../workdir/shaders/test_pass/test.vert", shaderc_glsl_vertex_shader);
        auto frag_code = VulkanUtil::compileFile("../workdir/shaders/test_pass/test.frag", shaderc_glsl_fragment_shader);

        VkShaderModule vert_shader_module = VulkanUtil::createShaderModule(ctx.m_device, vert_code);
        VkShaderModule frag_shader_module = VulkanUtil::createShaderModule(ctx.m_device, frag_code);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vert_shader_module;
        vertShaderStageInfo.pName = "main";
        vertShaderStageInfo.pSpecializationInfo = nullptr;

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = frag_shader_module;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        auto bindingDesc = VertexWithoutTexcoord::getBindingDescription();
        auto attriDesc = VertexWithoutTexcoord::getAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = attriDesc.size();
        vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
        vertexInputInfo.pVertexAttributeDescriptions = attriDesc.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        // VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
        // VK_PRIMITIVE_TOPOLOGY_LINE_LIST : line from every 2 vertices without reuse
        // VK_PRIMITIVE_TOPOLOGY_LINE_STRIP : the end vertex of every line is used as start vertex for the next line
        // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : triangle from every 3 vertices without reuse
        // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : the second and third vertex of every triangle are used as first two vertices of the next triangle
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)ctx.m_swapchainExtent.width;
        viewport.height = (float)ctx.m_swapchainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = ctx.m_swapchainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        // VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
        // VK_POLYGON_MODE_LINE : polygon edges are drawn as lines
        // VK_POLYGON_MODE_POINT : polygon vertices are drawn as points
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        //rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        //rasterizer.depthBiasClamp = 0.0f; // Optional
        //rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        //multisampling.minSampleShading = 1.0f; // Optional
        //multisampling.pSampleMask = nullptr; // Optional
        //multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        //multisampling.alphaToOneEnable = VK_FALSE; // Optional

        // After a fragment shader has returned a color
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        //colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        //colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        //colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        //colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        //colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        //colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(ctx.m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional

        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_ci {};
        dynamic_state_ci.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_ci.dynamicStateCount = 2;
        dynamic_state_ci.pDynamicStates    = dynamic_states;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pDynamicState = &dynamic_state_ci;

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(ctx.m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline));

        // ---------------------clean up shader-------------------------------
        vkDestroyShaderModule(ctx.m_device, frag_shader_module, nullptr);
        vkDestroyShaderModule(ctx.m_device, vert_shader_module, nullptr);
    }

    void VulkanTestPass::_createFramebuffers(){
        auto& ctx = *m_vkContext;
        m_swapchainFramebuffers.resize(ctx.m_swapchainImageViews.size());

        // create frame buffer for every imageview
        for (size_t i = 0; i < ctx.m_swapchainImageViews.size(); ++i)
        {
            std::array<VkImageView, 2> attachments = {
                ctx.m_swapchainImageViews[i]
                , ctx.m_depthImageView
            };

            VkFramebufferCreateInfo framebuffer_ci{};
            framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_ci.renderPass = m_renderPass;
            framebuffer_ci.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebuffer_ci.pAttachments = attachments.data();
            framebuffer_ci.width = ctx.m_swapchainExtent.width;
            framebuffer_ci.height = ctx.m_swapchainExtent.height;
            framebuffer_ci.layers = 1;

            VK_CHECK_RESULT(vkCreateFramebuffer(ctx.m_device, &framebuffer_ci, nullptr, &m_swapchainFramebuffers[i]));
        }
    }
}