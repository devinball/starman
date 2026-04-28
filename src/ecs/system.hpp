#pragma once

#include "core/context.hpp"
#include "core/command_buffer.hpp"

struct System {
  public:
    Context context;
    virtual void init() {};
    virtual void update() {};
    virtual void draw() {};
};
