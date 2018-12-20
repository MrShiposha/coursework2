#include "texture.h"

Texture::~Texture()
{
    destroy();
}

void Texture::update_descriptor()
{
    descriptor.sampler     = sampler;
    descriptor.imageView   = view;
    descriptor.imageLayout = image_layout;
}

void Texture::destroy()
{
    vkDestroyImageView(*device, view, nullptr);
    vkDestroyImage(*device, image, nullptr);
    if(sampler != nullptr)
    {
        vkDestroySampler(*device, sampler, nullptr);
        sampler = nullptr;
    }
    
    vkFreeMemory(*device, device_memory, nullptr);
}