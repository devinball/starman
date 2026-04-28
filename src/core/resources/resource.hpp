#pragma once

#include <string>

struct Resource {
  private:
    std::string id;
    bool loaded = false;
  public:
    Resource(const std::string& id) : id(id) {}
    virtual ~Resource() = default;

    const std::string& getId() const { return id; }
    bool isLoaded() const { return loaded; }
    bool isValid() const { return id != ""; } // lazy check, in future should check if it actually references a valid file

    virtual bool load() { return false; }
    virtual bool unload() { return false; }
};