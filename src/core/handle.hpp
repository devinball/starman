#pragma once

#include <string>

template<typename T>
struct Handle {
private:
    std::string id;
public:
    Handle() = default;
    explicit Handle(const std::string& id) : id(id) {}

    bool isValid()              const { return !id.empty(); }
    operator bool()             const { return isValid(); }
    const std::string& getId()  const { return id; }

    bool operator==(const Handle&) const = default;
};