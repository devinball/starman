#include "core/application.hpp"

int main()
{
  printf("\n-----STARTING-----\n\n");

  auto app = Application{};
  app.run();

  // may need to add layers for UI (via IMGUI)
  // add entities to the scene
  // add systems for gravity, thrust, navigation, trajectories, etc

  printf("\n-----ENDING-----\n\n");

  return 0;
}
