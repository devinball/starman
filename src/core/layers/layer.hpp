#pragma once

#include "core/events.hpp"
#include "core/context.hpp"

#include <memory>

struct Layer {
  public:
    std::shared_ptr<Context> context;
    virtual void update() {}
    virtual void draw() {}
    virtual void init() {}
    virtual void shutdown() {}
    virtual void event(Event event) {}
    virtual void setContext(Context ctx) {}
};
