#pragma once

#include "core/resources/resource.hpp"
#include "core/handle.hpp"

#include <map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <memory>

struct ResourcePool
{
private:
  struct ResourceData
  {
    std::shared_ptr<Resource> resource;
    int refCount = 0;
  };

  std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> pool;

public:
  template <typename T>
  Handle<T> load(const std::string &id)
  {
    static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");

    auto &typeMap = pool[std::type_index(typeid(T))];
    auto it = typeMap.find(id);

    if (it != typeMap.end())
    {
      it->second.refCount++;
      return Handle<T>(id);
    }

    auto resource = std::make_shared<T>(id);
    if (!resource->load())
      return Handle<T>(); // invalid handle — load failed

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
};
