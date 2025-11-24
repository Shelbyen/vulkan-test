#include "vk_engine.h"

// #include <SDL.h>
// #include <SDL_vulkan.h>

// #include <vk_initializers.h>
#include "vk_types.h"

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

    // everything went fine
    _isInitialized = true;
}

void VulkanEngine::cleanup()
{
    if (_isInitialized) {
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
