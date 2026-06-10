#pragma once

#include "core/resources/resource.hpp"

#include <map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <memory>


/*
struct ResourcePool
{
private:
  struct ResourceData
  {
    std::shared_ptr<Resource> resource;
    int refCount = 0;
  };

  std::unordered_map<std::type_index, std::unordered_map<ResourceID, ResourceData>> pool;
  std::unordered_map<std::string, ResourceID> idMap;

public:
  template <typename T>
  Handle<T> load(const std::string &source)
  {
    static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

    ResourceID id = getId(&source);

    auto &typeMap = pool[std::type_index(typeid(T))];
    auto it = typeMap.find(id);

    if (it != typeMap.end())
    {
      it->second.refCount++;
      return Handle<T>(id);
    }

    id = newId();

    auto resource = std::make_shared<T>(id, source);
    if (!resource->load())
      return Handle<T>(0); // invalid handle — load failed

    typeMap[id] = {resource, 1};

    return Handle<T>(id);
  }

  template <typename T>
  T *get(const Handle<T> &handle) const
  {
    if (!handle)
      return nullptr;

    auto typeIt = pool.find(std::type_index(typeid(T)));
    if (typeIt == pool.end())
      return nullptr;

    auto resIt = typeIt->second.find(handle.getId());
    if (resIt == typeIt->second.end())
      return nullptr;

    // static_cast is safe — we only store T under typeid(T)
    return static_cast<T *>(resIt->second.resource.get());
  }

  template <typename T>
  void unload(const Handle<T> &handle)
  {
    if (!handle)
      return;

    auto typeIt = pool.find(std::type_index(typeid(T)));
    if (typeIt == pool.end())
      return;

    auto resIt = typeIt->second.find(handle.getId());
    if (resIt == typeIt->second.end())
      return;

    auto &data = resIt->second;
    if (--data.refCount <= 0)
    {
      data.resource->unload();
      typeIt->second.erase(resIt);
    }
  }

  template <typename T>
  bool has(const Handle<T> &handle) const
  {
    return get(handle) != nullptr;
  }

  ResourceID getId(std::string& source) {
    // return 0 if cannot find
    auto it = idMap.find(source);
    if (it == idMap.end()) {
      return 0;
    }
    return it;
  }
};
*/

struct ResourceManager {
  private:
    std::unordered_map<ResourceID, std::shared_ptr<Resource>> resources;

    template<typename... Args>
    uint64_t hashArgs(Args&&... args) {
      uint64_t seed = 0;
      (..., (seed ^= (std::hash<std::decay_t<Args>>{}(args) + 0x9e3779b9 + (seed << 6) + (seed >> 2))));
      return seed;
    }

  public:
    template <typename T, typename... Args>
    Handle<T> load(Args&&... args) {
      // create id from a hash of the arguments
      ResourceID id = hashArgs(std::forward<Args>(args)...);

      auto it = resources.find(id);
      // if we have already seen this resource just return it
      if (it != resources.end()) {
        return Handle<T>(id);
        //return std::static_pointer_cast<T>(it->second);
      }

      auto resource = std::make_shared<T>(id);
      resource->load(std::forward<Args>(args)...);

      resources[id] = resource;
      return Handle<T>(id);
    }

    template <typename T>
    T* get(Handle<T> handle) {
      auto it = resources.find(handle.id);
      if (it != resources.end()) {
        return static_cast<T *>(it->second.get());
      }
      throw std::runtime_error("Invalid resource handle");
      // if we didn't find it return an invalid ID
      return 0; // return something?
    }
};

// maybe i would be able to load a resource with arbitrary parameters
// like resourceManager->load<Image>("a/texture.png");
// or resourceManager->load<Mesh>(procedual.SPHERE, 10);
// resourceManager->load<Mesh>("a/mesh.obj");
// it would then hash parameters
