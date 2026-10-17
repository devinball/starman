#pragma once
// take a scene.toml file and load it into an actual scene
// a scene.toml can reference prefabs

#include "ecs/ecs.hpp"
#include "core/context.hpp"
#include "resources/scene.hpp"
#include "math/utilities.hpp"

#include "ecs/systems/render_system.hpp"
#include "ecs/systems/orbit_system.hpp"
#include "ecs/systems/camera_system.hpp"
#include "ecs/systems/orbit_system.hpp"
#include "ecs/systems/icon_system.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/mesh_renderer.hpp"
#include "ecs/components/rigidbody.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/camera_controller.hpp"
#include "ecs/components/orbiter.hpp"
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

    Entity createPlanet(const std::string material, std::string name, double radius, double mass, double semiMajorAxis, double eccentricity, double incl, double longNode, double argPeri, double soiRadius, Entity parent) {
      Entity entity = context->registry->create();
      context->registry->emplace<Spatial>(entity, Vector3N{0, 0, 0}, Vector3F{float(radius), float(radius), float(radius)}, eulerToQuaternion(180, 0, 0));
      Handle<Mesh> mesh = context->resourceManager->load<Mesh>("example/models/uv_sphere.obj");
      Handle<Material> mat = context->resourceManager->load<Material>(material);
      context->registry->emplace<MeshRenderer>(entity, mesh, mat);

      // mass, eccentricity, semiMajorAxis, radians(incl), radians(longAsN), radians(argPeri), radians(longPeri), orbPer, bary, parent
      double systemMass = parent == entt::null ? mass : context->registry->get<CelestialBody>(parent).mass + mass; // yeah, sure?

      if (soiRadius == 0) {
        soiRadius = semiMajorAxis * pow(mass / systemMass, 0.4);
      }

      //double orbPer = sqrt((pow(semiMajorAxis, 3) * 4 * pow(std::numbers::pi, 2)) / (6.6743015e-11 * centerMass));

      // TODO: this ends at infinity, unsure how bad this is
      Trajectory trajectory = {Segment(INFINITY, EndType::HORIZON, parent, KeplerianOrbit(
        eccentricity, semiMajorAxis * (1 - pow(eccentricity, 2)),
        radians(incl), radians(longNode), radians(argPeri), 0,
        systemMass * Constants::G
      ))};

      context->registry->emplace<CelestialBody>(entity, mass, mass * Constants::G, soiRadius);
      context->registry->emplace<Orbiter>(entity, trajectory);
      context->registry->emplace<Icon>(entity, Color(randomF(), randomF(), randomF(), 1.0f));
      context->registry->emplace<Tag>(entity, name);

      return entity;
    }

    float randNorm() {
      return randomF() * 2 - 1;
    }

    float signRand() {
      float x = randNorm();
      if (x > 0) return 1;
      if (x < 0) return -1;
      return 0;
    }

    float edgeBiasedRandom() {
      return pow(randomF(), 0.1) * 2 - 1;
    }

    Entity createOrbiter() {
      double pSpread = 5.79091e7 * 1e3 * (1 - pow(0.1, 2)) * (1 + (randomF() * 0.1)); //1e11;
      double vSpread = 47.36*1e3;

      Entity e = context->registry->create();
      //Handle<Mesh> mesh = context->resourceManager->load<Mesh>("example/models/cube.obj");//("example/models/iss.obj");
      //Handle<Material> material = context->resourceManager->load<Material>("example/materials/fabric.yaml");
      //context->registry->emplace<MeshRenderer>(e, mesh, material);
      float theta = 2 * Constants::pi * randomF();
      context->registry->emplace<Spatial>(e, Vector3N{0, 0, 0}, Vector3F{1, 1, 1}, QuaternionF());
      context->registry->emplace<Icon>(e, Color(randomF(), randomF(), randomF(), 1.0f));
      //context->registry->emplace<Tag>(e, "orbiter");

      Vector3N position = Vector3N{pSpread * std::cos(theta), pSpread * std::sin(theta), 1e5 * randNorm()};
      Vector3N velocity = Vector3N(std::sin(theta) * vSpread + randNorm() * 150, -1 * std::cos(theta) * vSpread + randNorm() * 150, randNorm() * 150);
      double timeOffset = 0;// int(randomF() * 3600); // hack to stop all orbits from being computed at once
      Trajectory trajectory = {Segment(5, EndType::HORIZON, entt::null, KinematicOrbit(Vector3N(0, 0, 0), Vector3N(0, 0, 0), Vector3N(0, 0, 0), 0))};

      context->registry->emplace<Orbiter>(e, trajectory);

      return e;
    }

    void createLight(Vector3N pos) {
      Entity entity = context->registry->create();
      context->registry->emplace<Spatial>(entity, pos, Vector3F{0, 0, 0}, QuaternionF());
      context->registry->emplace<PointLight>(entity, 3.86e26, 2e24, Color(1, 1, 1)); //Color(0.99,0.9,0.44,0)

      // p = 3.86e24
      // r = 200e24
    }

    void solSystem() {
      createLight({ 0, 0, 0});
      //createLight({ 5e14, 0, 0});

      // TODO: bary centric coordinates

      Entity sun = createPlanet("example/materials/planets/sun.yaml", "sun", 696000000, 1.988416e30, 0.1, 0.1, 0, 0, 0, 1e15, entt::null); // sun
      //createPlanet("example/materials/planets/sun.yaml", "sun 2", 496000000, 1.125e30, 7.78321e9, 0.04846, 1.29861416, 14.27495244, 100.29282654, 0, sun); // sun 2
      createPlanet("example/materials/planets/mecury.yaml", "mecury", 2440530, 0.330103e24, 5.79091e7 * 1e3, 0.2000000564, 7.00559432, 48.33961819, 77.45771895, 0, sun); // mecury
      createPlanet("example/materials/planets/venus.yaml", "venus", 6051800, 4.86731e24, 1.08209e8 * 1e3, 0.00676, 3.39777545, 76.67261496, 131.76755713, 0, sun); // venus
      Entity earth = createPlanet("example/materials/planets/earth.yaml", "earth", 6378137, 5.97217e24, 1.49598e8 * 1e3, 0.01670, -0.00054346, -5.11260389, 102.93005885, 0, sun); // earth
        createPlanet("example/materials/planets/moon.yaml", "moon", 1737400, 7.346e22, 3.83398e5 * 1e3, 0.05555, 0, 0, 0, 0, earth); // moon
      createPlanet("example/materials/planets/mars.yaml", "mars", 3396190, 0.641691e24, 2.27939e8 * 1e3, 0.09342, 1.85181869, 49.71320984, -23.91744784, 0, sun); // mars
      createPlanet("example/materials/planets/jupiter.yaml", "jupiter", 71492000, 1898.125e24, 7.78321e8 * 1e3, 0.04846, 1.29861416, 100.29282654, 14.27495244, 0, sun); // jupiter
      createPlanet("example/materials/planets/saturn.yaml", "saturn", 60268000, 568.317e24, 1.4291e9 * 1e3, 0.05468, 2.49424102, 113.63998702, 92.86136063, 0, sun); // saturn
      createPlanet("example/materials/planets/uranus.yaml", "uranus", 25559000, 86.8099e24, 2.87479e9 * 1e3, 0.04739, 0.77298127, 73.96250215, 172.43404441, 0, sun); // uranus
      createPlanet("example/materials/planets/neptune.yaml", "neptune", 24764000, 102.4092e24, 4.50489e9 * 1e3, 0.00911, 1.77005520, 131.78635853, 46.68158724, 0, sun); // neptune


      createPlanet("example/materials/planets/mecury.yaml", "???", 2440530, 1e29, 5.79091e7 * 1e3, 0, 0, 0, 0, 0, sun);

      //createPlanet("example/materials/planets/sun.yaml", 482100, 936.416e18, e9 * 1e3, ); // ceres
      //createPlanet("example/materials/planets/sun.yaml", 1188300, 12024.6e18, 5.91540e9 * 1e3, 0.24906); // pluto
      //createPlanet("example/materials/planets/sun.yaml", 1200000, 16600e18, e9 * 1e3, ); // eris
      //createPlanet("example/materials/planets/sun.yaml", 717000, 3100e18, e9 * 1e3, ); // makemake
      //createPlanet("example/materials/planets/sun.yaml", 870000, 4006e18, e9 * 1e3, ); // haumea
    }
    
    void fpsCam() {
      Entity camera = context->registry->create();
      context->registry->emplace<Spatial>(camera, Vector3N(0, 0, 0), Vector3F{1, 1, 1}, QuaternionF());
      context->registry->emplace<Camera>(camera, true, 0, 0, 90.f, 0.1f, 10000.f, Color(0+0.0, 0+0.0, 0.01+0.0, 1));
      context->registry->emplace<CameraController>(camera, 30, 1.f); //200 good default for space, 1 for other
    }

    void init() {
      //YAML::Node config = YAML::LoadFile("example/scenes/simple.yaml");

      fpsCam();
      solSystem();

      for (int i = 0; i < 1000; i++) {
        createOrbiter();
      }

      addSystem<RenderSystem>();
      addSystem<OrbitSystem>();
      addSystem<CameraSystem>();
      addSystem<IconSystem>();

      for (auto& system : systems) {
        system->context = context;
      }
      

      for (auto& system : systems) {
        system->init();
      }
    }

    int counter = 0;

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
