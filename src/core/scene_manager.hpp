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
#include "ecs/systems/icon_system.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/mesh_renderer.hpp"
#include "ecs/components/rigidbody.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/camera_controller.hpp"
#include "ecs/components/keplerian_orbiter.hpp"
#include "ecs/components/icon.hpp"
#include "ecs/components/tag.hpp"

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

    Entity createPlanet(const std::string material, std::string name, double radius, double mass, double semiMajorAxis, float eccentricity, float incl, float longPeri, float longNode, double centerMass, Entity parent) {
      Entity entity = context->registry->create();
      context->registry->emplace<Spatial>(entity, Vector3N{0, 0, 0}, Vector3F{float(radius), float(radius), float(radius)}, eulerToQuaternion(180, 0, 0));
      Handle<Mesh> mesh = context->resourceManager->load<Mesh>("example/models/uv_sphere.obj");
      Handle<Material> mat = context->resourceManager->load<Material>(material);
      context->registry->emplace<MeshRenderer>(entity, mesh, mat);
      double argPeri = 0;
      double longAsN = longNode;
      double orbPer = sqrt((pow(semiMajorAxis, 3) * 4 * pow(std::numbers::pi, 2)) / (6.6743015e-11 * centerMass));

      //printf("%s %f \n", material.c_str(), orbPer / 3600);

      Vector3N bary = Vector3N(0, 0, 0);
      context->registry->emplace<KeplerianOrbiter>(entity, mass, eccentricity, semiMajorAxis, radians(incl), radians(longAsN), radians(argPeri), radians(longPeri), orbPer, bary, parent);
      context->registry->emplace<Icon>(entity, Color(randomF(), randomF(), randomF(), 1.0f));
      context->registry->emplace<Tag>(entity, name);

      return entity;
    }

    void createLight(Vector3N pos) {
      Entity entity = context->registry->create();
      context->registry->emplace<Spatial>(entity, pos, Vector3F{0, 0, 0}, QuaternionF());
      context->registry->emplace<PointLight>(entity, 3.86e26, 2e24, Color(1, 1, 1)); //Color(0.99,0.9,0.44,0)

      // p = 3.86e24
      // r = 200e24
    }

    /*
    void loadSolarSystem(const std::string path) {
      YAML::Node system = YAML::LoadFile("example/scenes/simple.yaml");

      Vector3N barycenter = system['solar_system']['barycenter'];

      for (auto body : system['solar_system']['bodies']) {
        std::string name = body['name'];
        std::string material = body['material'];
        double radius = body['radius'];
        double mass = body['mass'];
        double semiMajorAxis = body['semiMajorAxis'];
        double eccentricity = body['eccentricity'];
        double inclination = body['inclination'];
        double longitudePeriapsis = body['longitudePeriapsis'];
        double longitudeAscendingNode = body['longitudeAscendingNode'];
        std::string orbits = body['orbits'];

        Entity parent;
        if (orbits == "barycenter") {a
          parent = Entt::
        }

        createPlanet(material,
          name,
          radius,
          mass,
          semiMajorAxis,
          eccentricity,
          inclination,
          longitudePeriapsis,
          longitudeAscendingNode,
          mass of system, entity that is being orbited
        );
      }
    }
    */

    void solSystem() {
      createLight({ 0, 0, 0});
      //createLight({ 5e14, 0, 0});

      // TODO: bary centric coordinates
      Entity sun = createPlanet("example/materials/planets/sun.yaml", "sun", 696000000, 1.988416e30, 0.1, 0.1, 0, 0, 0, 1.988416e30, entt::null); // sun
      //createPlanet("example/materials/planets/sun.yaml", "sun 2", 496000000, 1.125e30, 7.78321e9, 0.04846, 1.29861416, 14.27495244, 100.29282654, 1.988416e30, sun); // sun 2
      createPlanet("example/materials/planets/mecury.yaml", "mecury", 2440530, 0.330103e24, 5.79091e7 * 1e3, 0.2000000564, 7.00559432, 77.45771895, 48.33961819, 1.988416e30, sun); // mecury
      createPlanet("example/materials/planets/venus.yaml", "venus", 6051800, 4.86731e24, 1.08209e8 * 1e3, 0.00676, 3.39777545, 131.76755713, 76.67261496, 1.988416e30, sun); // venus
      Entity earth = createPlanet("example/materials/planets/earth.yaml", "earth", 6378137, 5.97217e24, 1.49598e8 * 1e3, 0.01670, -0.00054346, 102.93005885, -5.11260389, 1.988416e30, sun); // earth
        createPlanet("example/materials/planets/moon.yaml", "moon", 1737400, 7.346e22, 3.83398e5 * 1e3, 0.05555, 0, 0, 0, 5.97217e24, earth); // moon
      createPlanet("example/materials/planets/mars.yaml", "mars", 3396190, 0.641691e24, 2.27939e8 * 1e3, 0.09342, 1.85181869, -23.91744784, 49.71320984, 1.988416e30, sun); // mars
      createPlanet("example/materials/planets/jupiter.yaml", "jupiter", 71492000, 1898.125e24, 7.78321e8 * 1e3, 0.04846, 1.29861416, 14.27495244, 100.29282654, 1.988416e30, sun); // jupiter
      createPlanet("example/materials/planets/saturn.yaml", "saturn", 60268000, 568.317e24, 1.4291e9 * 1e3, 0.05468, 2.49424102, 92.86136063, 113.63998702, 1.988416e30, sun); // saturn
      createPlanet("example/materials/planets/uranus.yaml", "uranus", 25559000, 86.8099e24, 2.87479e9 * 1e3, 0.04739, 0.77298127 , 172.43404441, 73.96250215, 1.988416e30, sun); // uranus
      createPlanet("example/materials/planets/neptune.yaml", "neptune", 24764000, 102.4092e24, 4.50489e9 * 1e3, 0.00911, 1.77005520, 46.68158724, 131.78635853, 1.988416e30, sun); // neptune

      //createPlanet("example/materials/planets/sun.yaml", 482100, 936.416e18, e9 * 1e3, ); // ceres
      //createPlanet("example/materials/planets/sun.yaml", 1188300, 12024.6e18, 5.91540e9 * 1e3, 0.24906); // pluto
      //createPlanet("example/materials/planets/sun.yaml", 1200000, 16600e18, e9 * 1e3, ); // eris
      //createPlanet("example/materials/planets/sun.yaml", 717000, 3100e18, e9 * 1e3, ); // makemake
      //createPlanet("example/materials/planets/sun.yaml", 870000, 4006e18, e9 * 1e3, ); // haumea
    }

    void frog(int num) {
      for (int i = 0; i < num; i++) {
        Entity entity = context->registry->create();
        context->registry->emplace<Spatial>(entity, Vector3N{randomF() * 100, randomF() * 100, randomF() * 100}, Vector3F{10, 10, 10}, eulerToQuaternion(180, 0, 0));
        Handle<Mesh> mesh = context->resourceManager->load<Mesh>("example/models/simple_frog.obj");
        Handle<Material> mat = context->resourceManager->load<Material>("example/materials/fabric.yaml");
        context->registry->emplace<MeshRenderer>(entity, mesh, mat);
      }
    }

    Entity createRigidbody(Vector3N position) {
      Entity e = context->registry->create();
      Handle<Mesh> mesh = context->resourceManager->load<Mesh>("example/models/iss.obj");
      Handle<Material> material = context->resourceManager->load<Material>("example/materials/fabric.yaml");
      context->registry->emplace<MeshRenderer>(e, mesh, material);
      context->registry->emplace<Spatial>(e, position, Vector3F{1, 1, 1}, QuaternionF());
      context->registry->emplace<Rigidbody>(e, 10000, 1000);
      context->registry->emplace<Icon>(e, Color(randomF(), randomF(), randomF(), 1.0f));
      context->registry->emplace<Tag>(e, "International Space Station");

      return e;
    }

    void fpsCam() {
      Entity camera = context->registry->create();
      context->registry->emplace<Spatial>(camera, Vector3N(0, 0, 0), Vector3F{1, 1, 1}, QuaternionF());
      context->registry->emplace<Camera>(camera, true, 0, 0, 90.f, 0.1f, 10000.f, Color(0+0.0, 0+0.0, 0.01+0.0, 1));
      context->registry->emplace<CameraController>(camera, 30, 1.f); //200 good default for space, 1 for other
    }

    void init() {
      YAML::Node config = YAML::LoadFile("example/scenes/simple.yaml");

      fpsCam();
      //fpc2();
      solSystem();

      for (int i = 0; i < 100; ++i) {
        createRigidbody({(i+5) * 1e10, 0, 0});
      }

      //createLight({0, 0, 0});

      addSystem<RenderSystem>();
      addSystem<PhysicsSystem>();
      addSystem<CameraSystem>();
      addSystem<IconSystem>();

      for (auto& system : systems) {
        system->context = context;
      }
      

      for (auto& system : systems) {
        system->init();
      }
    }

    void update() {
      for (auto& system : systems) {
        system->update();
      }
    }

    void draw() {
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
