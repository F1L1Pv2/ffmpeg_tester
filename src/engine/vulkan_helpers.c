#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_helpers.h"

int getNeededQueueFamilyIndex(VkQueueFlags flags){
    for(int i = 0; i < multipleQueueFamilyProperties.count; i++){
        if(multipleQueueFamilyProperties.items[i].queueFlags & flags){
            return i;
        }
    }
    return -1;
}

bool findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* index) {
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            *index = i;
            return true;
        }
    }
    return false;
}

VkCommandBuffer beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void vkCmdBeginRenderingEX_opt(VkCommandBuffer commandBuffer, BeginRenderingEX args){
    VkRenderingAttachmentInfo colorAttachment = {0};
    if(args.colorAttachment != NULL){
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = args.colorAttachment;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = args.clearBackground ? VK_ATTACHMENT_LOAD_OP_CLEAR : 0;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color.float32[0] = args.clearColor.r;
        colorAttachment.clearValue.color.float32[1] = args.clearColor.g;
        colorAttachment.clearValue.color.float32[2] = args.clearColor.b;
        colorAttachment.clearValue.color.float32[3] = args.clearColor.a;
    }

    VkRenderingAttachmentInfo depthAttachment = {0};
    if(args.depthAttachment != NULL){
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = args.depthAttachment;
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.clearValue.depthStencil = (VkClearDepthStencilValue){1.0f,0.0};
    }
    
    if(args.renderArea.width == 0 && args.renderArea.height == 0) args.renderArea = swapchainExtent;

    VkRenderingInfo renderingInfo = {0};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = (VkOffset2D){0};
    renderingInfo.renderArea.extent = args.renderArea;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = args.colorAttachment != NULL ? 1 : 0;
    renderingInfo.pColorAttachments = args.colorAttachment != NULL ? &colorAttachment : NULL;
    renderingInfo.pDepthAttachment = args.depthAttachment != NULL ? &depthAttachment : NULL;
    renderingInfo.pStencilAttachment = NULL;

    vkCmdBeginRendering(commandBuffer, &renderingInfo);
}