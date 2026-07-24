#pragma once

#include "ecs/system.hpp"
#include "ecs/components/keplerian_orbiter.hpp"
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



  public:
    

    
  
    void update() {
      
    }
};
