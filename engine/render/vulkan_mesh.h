#ifndef GALAXYSAILING_VULKAN_MESH_H_
#define GALAXYSAILING_VULKAN_MESH_H_

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>
namespace galaxysailing {
    struct VertexWithoutTexcoord
    {
        glm::vec3 pos;
        glm::vec3 normal;

        static VkVertexInputBindingDescription getBindingDescription(){
            VkVertexInputBindingDescription binding_desc{};
            binding_desc.binding = 0;
            binding_desc.stride = sizeof(VertexWithoutTexcoord);
            binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return binding_desc;
        }

        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions(){
            std::array<VkVertexInputAttributeDescription, 2> attribute_descs{};

            attribute_descs[0].binding = 0;
            attribute_descs[0].location = 0;
            attribute_descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descs[0].offset = offsetof(VertexWithoutTexcoord, pos);

            attribute_descs[1].binding = 0;
            attribute_descs[1].location = 1;
            attribute_descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descs[1].offset = offsetof(VertexWithoutTexcoord, normal);

            return attribute_descs;
        }
    };
    
}

#endif