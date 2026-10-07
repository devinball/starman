#pragma once

#include "ecs/system.hpp"
#include "ecs/components/orbiter.hpp"
#include "ecs/components/spatial.hpp"

#include "imgui.h"

struct KeplerianOrbitSystem : System {
  private:
    double guess(double M, double e) {
      double n = std::sqrt(5 + std::sqrt(16 + 9/e));
      double a = n*(e*(n*n - 1) + 1)/6;
      double c = n*(1-e);
      double d = -M;
      double p = c/a;
      double q = d/a;
      double k = std::sqrt(q*q/4 + p*p*p/27);
      double s = std::cbrt(-q/2 - k) + std::cbrt(-q/2 + k);
      return n*std::asin(s);
    }
  
    void update() {
      // SOIs are determined by celestial body objects

      sphereOfInfluenceRadius = a * pow(m / M, 2/5);

      auto orbiterView = context->registry->view<Orbiter, Spatial>();
      orbiterView.each([this](auto &orbiter, auto &spatial){
        spatial.position = orbiter.trajectory.position(context->simTime);
        // potentially use orbiter.trajectory.velocity(context->simTime);
        // in order to interpolate between frames

      });
    }
};
