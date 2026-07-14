#pragma once
// take a scene.toml file and load it into an actual scene
// a scene.toml can reference prefabs

#include "ecs/ecs.hpp"
#include "core/context.hpp"
#include "resources/scene.hpp"
#include "math/utilities.hpp"

#include "ecs/systems/render_system.hpp"
#include "ecs/systems/physics_system.hpp"
#include "ecs/systems/camera_system.hpp"
#include "ecs/systems/keplerian_orbit_system.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/mesh_renderer.hpp"
#include "ecs/components/rigidbody.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/camera_controller.hpp"
#include "ecs/components/keplerian_orbiter.hpp"

#include <memory>
#include <vector>
#include <math.h>

// you can have multiple scenes!, why not!
// each scene keeps track of what entities it 'owns'
// that's all a scene does though, tracks entities.

// maybe i could use the onEvent better
// an event could be a draw call, a resource load or get request
// it could be a scene load, or a window close

// it might be kinda cool if scenes had basic 'macros'
// so you could specify to copy an object n times
// with its position varying by some amount per instance

struct SceneManager {
  private:
    std::vector<std::unique_ptr<System>> systems;
    std::vector<std::unique_ptr<Scene>> loadedScenes;
  public:
    std::shared_ptr<Context> context;

    void createPlanet(const std::string material, double radius, double mass, double semiMajorAxis, float eccentricity, float incl, float longPeri, float longNode) {
      Entity entity = context->registry->create();
      context->registry->emplace<Spatial>(entity, Vector3{0, 0, 0}, Vector3F{radius, radius, radius}, eulerToQuaternion(180, 0, 0));
      Handle<Mesh> mesh = context->resourceManager->load<Mesh>("example/models/uv_sphere.obj");
      Handle<Material> mat = context->resourceManager->load<Material>(material);
      context->registry->emplace<MeshRenderer>(entity, mesh, mat);
      double argPeri = 0;
      double longAsN = longNode;
      double orbPer = sqrt((pow(semiMajorAxis, 3) * 4 * pow(std::numbers::pi, 2)) / (0.000000000066743015 * 1989100e24));

      Vector3 bary = Vector3(0, 0, 0);
      context->registry->emplace<KeplerianOrbiter>(entity, 0, 0, eccentricity, semiMajorAxis, 0, 0, radians(incl), radians(longAsN), radians(argPeri), radians(longPeri), 0, orbPer, bary);
    }

    void createLight() {
      Entity entity = context->registry->create();
      context->registry->emplace<Spatial>(entity, Vector3{0, 0, 0}, Vector3F{0, 0, 0}, QuaternionF());
      context->registry->emplace<PointLight>(entity, 3.86e26, 2e24, Color(0.99,0.9,0.44,0));

      // p = 3.86e24
      // r = 200e24
    }

    void init() {
      YAML::Node config = YAML::LoadFile("example/scenes/simple.yaml");

      /*
      if (config["entities"]) {
        
      }
      */

      // TODO: High numbers of meshes do not preform as well as i would like

      Entity camera = context->registry->create();
      context->registry->emplace<Spatial>(camera, Vector3(0, 0, 0), Vector3F{1, 1, 1}, QuaternionF());
      context->registry->emplace<Camera>(camera, true, 0, 0, 90.f, 10000000000.f, 100000000000000000.f, Color(0, 0.2, 0.01, 1));
      context->registry->emplace<CameraController>(camera, 10000.f);

      createLight();

      //createPlanet("example/materials/planets/sun.yaml", 50.0 * 696000000, 1989100e24, 0.1, 0.1, 0, 0, 0); // sun
      createPlanet("example/materials/planets/mecury.yaml", 700.0 * 2440530, 0.330103e24, 5.79091e7 * 1e3, 0.2000000564, 7.00559432, 77.45771895, 48.33961819); // mecury
      createPlanet("example/materials/planets/venus.yaml", 700.0 * 6051800, 4.86731e24, 1.08209e8 * 1e3, 0.00676, 3.39777545, 131.76755713, 76.67261496); // venus
      createPlanet("example/materials/planets/earth.yaml", 700.0 * 6378137, 5.97217e24, 1.49598e8 * 1e3, 0.01670, -0.00054346, 102.93005885, -5.11260389); // earth
        //createPlanet("example/materials/planets/moon.yaml", 700.0 * radius, mass, 3.83398e5 * 1e3, 0.05555, 0, 0, 0); // moon
      createPlanet("example/materials/planets/mars.yaml", 700.0 * 3396190, 0.641691e24, 2.27939e8 * 1e3, 0.09342, 1.85181869, -23.91744784, 49.71320984); // mars
      createPlanet("example/materials/planets/jupiter.yaml", 100.0 * 71492000, 1898.125e24, 7.78321e8 * 1e3, 0.04846, 1.29861416, 14.27495244, 100.29282654); // jupiter
      createPlanet("example/materials/planets/saturn.yaml", 100.0 * 60268000, 568.317e24, 1.4291e9 * 1e3, 0.05468, 2.49424102, 92.86136063, 113.63998702); // saturn
      createPlanet("example/materials/planets/uranus.yaml", 500.0 * 25559000, 86.8099e24, 2.87479e9 * 1e3, 0.04739, 0.77298127 , 172.43404441, 73.96250215); // uranus
      createPlanet("example/materials/planets/neptune.yaml", 500.0 * 24764000, 102.4092e24, 4.50489e9 * 1e3, 0.00911, 1.77005520, 46.68158724, 131.78635853); // neptune

      //createPlanet("example/materials/planets/sun.yaml", 482100, 936.416e18, e9 * 1e3, ); // ceres
      //createPlanet("example/materials/planets/sun.yaml", 1188300, 12024.6e18, 5.91540e9 * 1e3, 0.24906); // pluto
      //createPlanet("example/materials/planets/sun.yaml", 1200000, 16600e18, e9 * 1e3, ); // eris
      //createPlanet("example/materials/planets/sun.yaml", 717000, 3100e18, e9 * 1e3, ); // makemake
      //createPlanet("example/materials/planets/sun.yaml", 870000, 4006e18, e9 * 1e3, ); // haumea

      for (int i = 0; i < 100; i++) {
        Entity entity = context->registry->create();
        context->registry->emplace<Spatial>(entity, Vector3{randomF() * 100, randomF() * 100, randomF() * 100}, Vector3F{10, 10, 10}, eulerToQuaternion(180, 0, 0));
        Handle<Mesh> mesh = context->resourceManager->load<Mesh>("example/models/simple_frog.obj");
        Handle<Material> mat = context->resourceManager->load<Material>("example/materials/fabric.yaml");
        context->registry->emplace<MeshRenderer>(entity, mesh, mat);
      }

      /*
      float d = 0.5;

      for (int i = 0; i < 400; ++i) {
        float m = randomF() * 250;
        Vector3 r = Vector3(200 * (randomF() * 2 - 1), 200 * (randomF() * 2 - 1), 200 * (randomF() * 2 - 1));
        Vector3 v = Vector3(randomF() * 2 - 1, randomF() * 2 - 1, randomF() * 2 - 1) * 10;
        if (i < 3) {
          m = 75000;
          r = r * 0.1;
        }

        if (i > 200 - 10) {
          m = 1250 * 5;
        }
        float s = std::cbrt(m) * 0.2;
        Entity e = context->registry->create();
        context->registry->emplace<Spatial>(e,
          r + shift,
          Vector3F{s, s, s},// + (Vector3F{1, 1, 1} * 3 * randomF()),
          eulerToQuaternion(randomF() * 360, randomF() * 360, randomF() * 360
        ));
        Handle<Mesh> mesh = context->resourceManager->load<Mesh>("example/models/simple_frog.obj");
        Handle<Material> material = context->resourceManager->load<Material>("example/materials/fabric.yaml");
        context->registry->emplace<MeshRenderer>(e, mesh, material);
        context->registry->emplace<Rigidbody>(e,
          m,
          v,
          Vector3(randomF(), randomF(), randomF())
        );
      }
      */

      addSystem<RenderSystem>();
      addSystem<PhysicsSystem>();
      addSystem<KeplerianOrbitSystem>();
      addSystem<CameraSystem>();

      for (auto& system : systems) {
        system->context = context;
      }
      
      // load scenethis
      // init all systems from registry
      for (auto& system : systems) {
        system->init();
      }
    }

    void update() {
      // loop all systems from registry on fixed timestep
      for (auto& system : systems) {
        system->update();
      }
    }

    void draw() {
      // loop all systems from registry on every frame
      for (auto& system : systems) {
        system->draw();
      }
    }

    template<typename T, typename... Args>
    T& addSystem(Args&&... args) {
      auto& ptr = systems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
      return static_cast<T&>(*ptr);
    }

    void loadScene(Handle<Scene> sceneHandle) {
      /*
      Scene* scene = resourceManager->get(sceneHandle);
      if (scene == nullptr) return;

      auto& entities = scene->getEntities();

      for (auto& entityData : entities) {
        const auto entity = registry.create();
        registry.emplace<Spatial>(entity);

      }


      // load everything
      */
    }

    void unloadScene(const std::string &id) {

    }
};
