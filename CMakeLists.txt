cmake_minimum_required(VERSION 3.31)

project("app")

set(EXECUTABLE_NAME ${PROJECT_NAME})
add_executable(${EXECUTABLE_NAME})

target_sources(${EXECUTABLE_NAME} PRIVATE 
    src/main.cpp
    src/application.cpp
    src/utils.cpp
    src/window.cpp
    src/instance.cpp
    src/implementations.cpp
    src/device.cpp
    src/swapchain.cpp
    src/graphics_pipeline.cpp
    src/renderer.cpp
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
    include/
)

find_package(Vulkan REQUIRED COMPONENTS volk shaderc_combined)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
    Vulkan::Vulkan
    Vulkan::volk
    Vulkan::shaderc_combined
    glfw
    glm::glm
)

target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_23)

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
    VK_NO_PROTOTYPES
    GLFW_INCLUDE_VULKAN
    ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
    VULKAN_VERSION=VK_API_VERSION_1_4

)