#pragma once

#include "core/events.hpp"
#include "core/context.hpp"

struct Layer {
  public:
    Context context;
    virtual void update() {}
    virtual void draw() {}
    virtual void init() {}
    virtual void shutdown() {}
    virtual void event(Event event) {}
    virtual void setContext(Context ctx) {}
};
