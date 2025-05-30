/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup gpu
 */

#include "vk_sampler.hh"
#include "vk_backend.hh"
#include "vk_context.hh"

#include "DNA_userdef_types.h"

namespace blender::gpu {
VKSampler::~VKSampler()
{
  free();
}

void VKSampler::create(const GPUSamplerState &sampler_state)
{
  BLI_assert(sampler_state.type != GPU_SAMPLER_STATE_TYPE_INTERNAL);
  BLI_assert(vk_sampler_ == VK_NULL_HANDLE);

  const VKDevice &device = VKBackend::get().device;

  VkSamplerCreateInfo sampler_info = {};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  /* Extend */
  sampler_info.addressModeU = to_vk_sampler_address_mode(sampler_state.extend_x);
  sampler_info.addressModeV = sampler_info.addressModeW = to_vk_sampler_address_mode(
      sampler_state.extend_yz);
  sampler_info.minLod = 0;
  sampler_info.maxLod = 0;

  if (sampler_state.type == GPU_SAMPLER_STATE_TYPE_PARAMETERS) {
    /* Apply filtering. */
    if (sampler_state.filtering & GPU_SAMPLER_FILTERING_LINEAR) {
      sampler_info.magFilter = VK_FILTER_LINEAR;
      sampler_info.minFilter = VK_FILTER_LINEAR;
    }
    if (sampler_state.filtering & GPU_SAMPLER_FILTERING_MIPMAP) {
      sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      sampler_info.minLod = 0;
      sampler_info.maxLod = 1000;
    }
    if ((sampler_state.filtering & GPU_SAMPLER_FILTERING_ANISOTROPIC) &&
        (U.anisotropic_filter > 1) &&
        (device.physical_device_features_get().samplerAnisotropy == VK_TRUE))
    {
      sampler_info.anisotropyEnable = VK_TRUE;
      sampler_info.maxAnisotropy = U.anisotropic_filter;
    }
  }
  else if (sampler_state.type == GPU_SAMPLER_STATE_TYPE_CUSTOM) {
    if (sampler_state.custom_type == GPU_SAMPLER_CUSTOM_ICON) {
      sampler_info.magFilter = VK_FILTER_LINEAR;
      sampler_info.minFilter = VK_FILTER_LINEAR;
      sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      sampler_info.minLod = 0;
      sampler_info.maxLod = 1;
    }
    else if (sampler_state.custom_type == GPU_SAMPLER_CUSTOM_COMPARE) {
      sampler_info.magFilter = VK_FILTER_LINEAR;
      sampler_info.minFilter = VK_FILTER_LINEAR;
      sampler_info.compareEnable = VK_TRUE;
      sampler_info.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    }
  }

  vkCreateSampler(device.vk_handle(), &sampler_info, nullptr, &vk_sampler_);
  debug::object_label(vk_sampler_, sampler_state.to_string().c_str());
}

void VKSampler::free()
{

  if (vk_sampler_ != VK_NULL_HANDLE) {
    const VKDevice &device = VKBackend::get().device;
    if (device.vk_handle() != VK_NULL_HANDLE) {
      vkDestroySampler(device.vk_handle(), vk_sampler_, nullptr);
    }
    vk_sampler_ = VK_NULL_HANDLE;
  }
}

}  // namespace blender::gpu
