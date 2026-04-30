//#include "core/application.hpp"
#include "iostream"

#include "core/layers/render/opengl/opengl_renderer.hpp"

#include <chrono>

int main() {
    printf("\n-----STARTING-----\n\n");

    auto renderer = OpenGLRenderer{};
    auto resourcePool = ResourcePool{};
    renderer.resourcePool = &resourcePool;

    renderer.init({800, 600, "engine"}, nullptr);

    while (!renderer.shouldClose()) {
        auto start = std::chrono::system_clock::now();
        renderer.beginFrame();
        renderer.render();
        renderer.endFrame();
        auto end = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_seconds = end-start;

        std::cout << elapsed_seconds.count() << std::endl;
    }

    renderer.shutdown();

    //auto app = Application{};
    //app.run();

    // may need to add layers for UI (via IMGUI)
    // add entities to the scene
    // add systems for gravity, thrust, navigation, trajectories, etc

    

    printf("\n-----ENDING-----\n\n");

    return 0;
}
