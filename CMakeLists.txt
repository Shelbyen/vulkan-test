cmake_minimum_required(VERSION 3.16)
project(vulkan-test)

set(CMAKE_CXX_STANDARD 20)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
# find_package(glm REQUIRED)

# SET(GLM_TEST_ENABLE OFF CACHE BOOL "GLM Build unit tests")
include(FetchContent)
FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        main #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan vk-bootstrap::vk-bootstrap)