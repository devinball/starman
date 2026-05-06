#include "core/application.hpp"
#include "iostream"
#include "core/rendering/opengl/opengl_renderer.hpp"
#include "core/math/utilities.hpp"
#include <chrono>

void noAbstractionTest();

int main()
{
  printf("\n-----STARTING-----\n\n");

  //noAbstractionTest();

  auto app = Application{};
  app.run();

  // may need to add layers for UI (via IMGUI)
  // add entities to the scene
  // add systems for gravity, thrust, navigation, trajectories, etc

  printf("\n-----ENDING-----\n\n");

  return 0;
}

void noAbstractionTest() {
  auto renderer = OpenGLRenderer{};

  auto resourcePool = std::make_shared<ResourcePool>();
  auto sceneGraph = std::make_shared<SceneGraph>();

  renderer.resourcePool = resourcePool;
  renderer.sceneGraph = sceneGraph;

  renderer.init({800, 600, "engine"});

  float t;
  float d = 0.5;

  while (!renderer.shouldClose()) {
    sceneGraph->clear(); // probably don't need to clear every frame
    for (int i = 0; i < 100; ++i) {
      sceneGraph->submitModel(
        resourcePool->load<Mesh>("example/models/simple_frog.stl"),
        Handle<Material>(),
        transformMatrix(Vector3F(std::sin(i * d) * std::sqrt(i) * d, 0, std::cos(i * d) * std::sqrt(i) * d), Vector3F(1, 1, 1), eulerToQuat(t, t, t))
      );
    }

    auto start = std::chrono::system_clock::now();
    renderer.beginFrame();
    renderer.render();
    renderer.endFrame();
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    //std::cout << (1.0 / elapsed_seconds.count()) << std::endl;

    t += (float)elapsed_seconds.count();
  }

  renderer.shutdown();
}