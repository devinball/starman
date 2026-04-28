#pragma once

#include "core/context.hpp"
#include "core/resource_pool.hpp"

struct ResourceLayer : Layer {
  private:
    ResourcePool* resourcePool;
  public:
    void setContext(Context ctx) {
      resourcePool = ctx.resourcePool;
    }
};
