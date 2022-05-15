## 1 CommandPool & CommandBuffer

![cmdpool&cmdbuffer](images/cmdpool%26cmdbuffer.png)

### 1.1 Create CommandPool

创建command pool 示例：
```cpp
VkCommandPoolCreateInfo pool_create_info{};
pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
pool_create_info.queueFamilyIndex = m_vkContext.m_queueFamilyIndices.graphicsFamily.value();
pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
VK_CHECK_RESULT(vkCreateCommandPool(m_vkContext.m_device, &pool_create_info, nullptr, &m_commandPool));
```

值得注意的是`VkCommandPoolCreateInfo`的`flags`字段对于创建command pool的作用，`flags`有以下几个值

- `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT`: 分配出来的command buffer将在较短的帧中被 **reset(详见[VkResetCommandPool](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkResetCommandPool.html))** 或者free
- `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`: 允许任意的command buffer被reset, 可以显示调用`vkResetCommandBuffer`，或在调用`vkBeginCommandBuffer`时被隐式地reset.
- `VK_COMMAND_POOL_CREATE_PROTECTED_BIT`: command buffer 被保护(?)


### 1.2 Create CommandBuffer

```cpp
VkCommandBufferAllocateInfo cmd_buffer_allocate_info{};
cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
cmd_buffer_allocate_info.commandPool = m_commandPool;
cmd_buffer_allocate_info.commandBufferCount = 1U;
VK_CHECK_RESULT(vkAllocateCommandBuffers(m_vkContext.m_device, &cmd_buffer_allocate_info, &m_commandBuffers[i]));
```

`VkCommandBufferAllocateInfo::level`:
- `VK_COMMAND_BUFFER_LEVEL_PRIMARY`
- `VK_COMMAND_BUFFER_LEVEL_SECONDARY`


### 1.3 Begin CommandBuffer

