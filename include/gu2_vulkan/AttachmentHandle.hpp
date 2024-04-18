//
// Project: GraphicsUtils2
// File: AttachmentHandle.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <vulkan/vulkan.h>


namespace gu2 {


struct AttachmentHandle {
    VkAttachmentDescription description; // TODO possibly change into pointers since these do not need to be replicated for each swap chain image
    VkAttachmentReference   reference;
    VkImageView             imageView;
    VkDevice                device; // if not VK_NULL_HANDLE, imageView will be deleted in the destructor (ugly hack but it'll do for now)

    AttachmentHandle(
        VkAttachmentDescription description = {},
        VkAttachmentReference reference = {},
        VkImageView imageView = VK_NULL_HANDLE,
        VkDevice device = VK_NULL_HANDLE
    ) :
        description (description),
        reference   (reference),
        imageView   (imageView),
        device      (device)
    {
    }
    AttachmentHandle(const AttachmentHandle&) = delete;
    AttachmentHandle(AttachmentHandle&& other) noexcept :
        description (other.description),
        reference   (other.reference),
        imageView   (other.imageView),
        device      (other.device)
    {
        other.imageView = VK_NULL_HANDLE;
        other.device = VK_NULL_HANDLE;
    }
    AttachmentHandle& operator=(const AttachmentHandle&) = delete;
    AttachmentHandle& operator=(AttachmentHandle&& other) noexcept
    {
        if (this == &other)
            return *this;

        if (imageView != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
            vkDestroyImageView(device, imageView, nullptr);

        description = other.description;
        reference = other.reference;
        imageView = other.imageView;
        device = other.device;

        other.imageView = VK_NULL_HANDLE;
        other.device = VK_NULL_HANDLE;

        return *this;
    }
    ~AttachmentHandle() {
        if (imageView != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
            vkDestroyImageView(device, imageView, nullptr);
    }
};


} // namespace gu2
