
#ifndef CG_SEM5_VKGETPROCADDRESSMACROS_H
#define CG_SEM5_VKGETPROCADDRESSMACROS_H

#include <stdexcept>

// Macro to get a procedure address based on a vulkan instance
#define VK_GET_INSTANCE_PROC_ADDR(inst, entrypoint)                        \
{                                                                       \
	fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetInstanceProcAddr(inst, "vk"#entrypoint)); \
	if (fp##entrypoint == NULL)                                         \
	{																    \
		throw std::runtime_error("Can't get instance proc addr");       \
	}                                                                   \
}

// Macro to get a procedure address based on a vulkan device
#define VK_GET_DEVICE_PROC_ADDR(dev, entrypoint)                           \
{                                                                       \
	fp##entrypoint = reinterpret_cast<PFN_vk##entrypoint>(vkGetDeviceProcAddr(dev, "vk"#entrypoint));   \
	if (fp##entrypoint == NULL)                                         \
	{																    \
		throw std::runtime_error("Can't get device proc addr");         \
	}                                                                   \
}

#endif // CG_SEM5_VKGETPROCADDRESSMACROS_H