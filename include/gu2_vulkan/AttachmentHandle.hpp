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
    VkAttachmentDescription description {}; // TODO possibly change into pointers since these do not need to be replicated for each swap chain image
    VkAttachmentReference   reference   {};
    VkImageView             imageView   {nullptr};
    VkExtent2D              imageExtent {};
    // TODO maybe introduce proper RAII facilities to handle creation and freeing of imageView
};


} // namespace gu2
