#pragma once

#include "core/context.hpp"
#include "core/resource_pool.hpp"
#include "core/layers/layer.hpp"

#include <memory>

struct ResourceLayer : Layer {
  private:
    std::shared_ptr<ResourcePool> resourcePool;
  public:
    void setContext(Context ctx) {
      resourcePool = ctx.resourcePool;
    }
};
