#pragma once

#include <string>

using ResourceID = uint64_t;

struct Resource {
  ResourceID id;
  virtual ~Resource() = default;

  Resource(ResourceID id) : id(id) {}

  ResourceID getId() const { return id; }
  bool isValid() const { return id != 0; } // lazy check, in future should check if it actually references a valid file

  // maybe load should take std::string &path
  // or maybe it should take like a list of arguments
  //virtual bool load() = 0;
  //virtual bool unload() = 0;
};

template<typename T>
struct Handle {
  ResourceID id;

  Handle() = default;
  explicit Handle(ResourceID id) : id(id) {}

  operator bool() const { return id != 0; }
  bool operator==(const Handle&) const = default;
};
