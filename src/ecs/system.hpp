#pragma once

#include "core/context.hpp"
#include "core/command_buffer.hpp"

#include <memory>

struct System {
  public:
    std::shared_ptr<Context> context;
    virtual void init() {};
    virtual void update() {};
    virtual void draw() {};
};
