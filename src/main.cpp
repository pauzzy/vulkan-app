#include "application.h"

int main(int argc, char* argv[]) {
    constexpr uint32_t WND_WIDTH  = 1200;
    constexpr uint32_t WND_HEIGHT = 800;
    constexpr std::string_view WND_TITLE = "Vulkan";

    try {
        Application app(
            WND_WIDTH, WND_HEIGHT, 
            WND_TITLE
        );
        app.run();
    } catch (const std::exception& e) {
        std::println("Error: {}", e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}