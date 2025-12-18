#include "vk_engine.h"
#include <iostream>

// #include <SDL.h>
// #include <SDL_vulkan.h>

// #include <vk_initializers.h>



#include <chrono>
#include <thread>


constexpr bool bUseValidationLayers = false;

VulkanEngine* loadedEngine = nullptr;

VulkanEngine& VulkanEngine::Get() { return *loadedEngine; }
void VulkanEngine::init()
{
    // only one engine initialization is allowed with the application.
    assert(loadedEngine == nullptr);
    loadedEngine = this;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // TODO: enable window resizable
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    _window = glfwCreateWindow(
        _windowExtent.width, 
        _windowExtent.height, 
        "Vulkan", 
        nullptr, nullptr);
    
    init_device();

	init_swapchain();

	init_commands();

	init_sync_structures();

    // everything went fine
    _isInitialized = true;
}

VkSurfaceKHR VulkanEngine::create_surface_glfw() {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkResult err = glfwCreateWindowSurface(_instance, _window, nullptr, &surface);
    if (err != VK_SUCCESS) {
        const char* error_msg;
        int ret = glfwGetError(&error_msg);
        if (ret != 0) {
            std::cout << ret << " ";
            if (error_msg != nullptr) std::cout << error_msg;
            std::cout << "\n";
        }
        surface = VK_NULL_HANDLE;
    }
    return surface;
}

int VulkanEngine::init_device() {
    vkb::InstanceBuilder instance_builder;
    auto instance_ret = instance_builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(bUseValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();

    if (!instance_ret) {
        std::cout << instance_ret.error().message() << "\n";
        return -1;
    }
    vkb::Instance vkb_instance = instance_ret.value();
    _instance = vkb_instance.instance;
    _surface = create_surface_glfw();

    //vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features.dynamicRendering = true;
	features.synchronization2 = true;

	//vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector phys_device_selector(vkb_instance);

    auto phys_device_ret = phys_device_selector
        .set_surface(_surface)
        .set_minimum_version(1, 1)
        .set_required_features_13(features)
		.set_required_features_12(features12)
        .select();
    if (!phys_device_ret) {
        std::cout << phys_device_ret.error().message() << "\n";
        if (phys_device_ret.error() == vkb::PhysicalDeviceError::no_suitable_device) {
            const auto& detailed_reasons = phys_device_ret.detailed_failure_reasons();
            if (!detailed_reasons.empty()) {
                std::cerr << "GPU Selection failure reasons:\n";
                for (const std::string& reason : detailed_reasons) {
                    std::cerr << reason << "\n";
                }
            }
        }
        return -1;
    }
    vkb::PhysicalDevice physical_device = phys_device_ret.value();

    vkb::DeviceBuilder device_builder{ physical_device };

    auto device_ret = device_builder.build();
    if (!device_ret) {
        std::cout << device_ret.error().message() << "\n";
        return -1;
    }
    vkb::Device vkbDevice = device_builder.build().value();
    _device = vkbDevice.device;
	_chosenGPU = physical_device.physical_device;

    return 0;
}


void VulkanEngine::create_swapchain(uint32_t width, uint32_t height)
{
	vkb::SwapchainBuilder swapchainBuilder{ _chosenGPU,_device,_surface };

	_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		//.use_default_format_selection()
		.set_desired_format(VkSurfaceFormatKHR{ .format = _swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build()
		.value();

	_swapchainExtent = vkbSwapchain.extent;
	//store swapchain and its related images
	_swapchain = vkbSwapchain.swapchain;
	_swapchainImages = vkbSwapchain.get_images().value();
	_swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanEngine::init_swapchain()
{
	create_swapchain(_windowExtent.width, _windowExtent.height);
}

void VulkanEngine::destroy_swapchain()
{
	vkDestroySwapchainKHR(_device, _swapchain, nullptr);

	// destroy swapchain resources
	for (int i = 0; i < _swapchainImageViews.size(); i++) {

		vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
	}
}


void VulkanEngine::init_commands()
{
    //nothing yet
}
void VulkanEngine::init_sync_structures()
{
    //nothing yet
}

void VulkanEngine::cleanup()
{
    if (_isInitialized) {
        destroy_swapchain();

		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyDevice(_device, nullptr);
		
		vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
		vkDestroyInstance(_instance, nullptr);
        
        glfwDestroyWindow(_window);

        glfwTerminate();
    }

    // clear engine pointer
    loadedEngine = nullptr;
}

void VulkanEngine::run()
{
    // SDL_Event e;
    bool bQuit = false;

    // main loop
    while (!bQuit) {
        // Handle events on queue
        glfwPollEvents();

        if (glfwWindowShouldClose(_window)) {
            bQuit = true;
        }

        // do not draw if we are minimized
        if (stop_rendering) {
            // throttle the speed to avoid the endless spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        draw();
    }
}

void VulkanEngine::draw()
{
    // nothing yet
}
