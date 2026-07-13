#pragma once

#include "ecs/system.hpp"
#include "ecs/components/keplerian_orbiter.hpp"
#include "ecs/components/spatial.hpp"

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

    double kepler(double E, double e) {
      return E - e * std::sin(E);
    }

    double eccentric_anomaly(double M, double e) {
      double E = M;
      //TODO: use guess instead for faster convergance
      
      double tolerance = 1e-10;
      for (int i = 0; i < 100; ++i) {
        double x = kepler(E, e) - M;
        if (std::abs(x) <= tolerance) {
          break;
        }
        E -= x / (1 - e * std::cos(E));
      }

      return E;
    }

    Vector3 keplerToCartesian(double E, double e, double a, double i, double w, double o) {      
      // a = semiMajorAxis
      // e = eccentiricty
      // i = inclination
      // l = mean longitude
      // u = longitude of periapsis
      // o = longitude of ascending node

      // https://ssd.jpl.nasa.gov/planets/approx_pos.html

      double r = a * (1 - e * std::cos(E));
      double b = e / (1 + std::sqrt(1 - e * e)); // beta, intermediate for finding true anomaly
      double f = E + 2 * std::atan((b * std::sin(E)) / (1 - b * std::cos(E))); // true anomaly, using R. Broucke and P. Cefola method

      Number x = r * (std::cos(o) * std::cos(w + f) - std::sin(o) * std::sin(w + f) * std::cos(i));
      Number y = r * (std::sin(o) * std::cos(w + f) + std::cos(o) * std::sin(w + f) * std::cos(i));
      Number z = r * std::sin(w + f) * std::sin(i);
      
      // coordinate schenagins
      return Vector3(
        x,
        z,
        y
      );
    }

  public:
    void update() {
      auto view = context->registry->view<KeplerianOrbiter, Spatial>();
      view.each([this](auto &keplerianOrbiter, auto &spatial) {
        double t = context->runtime * 3600 * 24 * 365 * 0.11; // 100 seconds = 1 year
        double M = 2 * std::numbers::pi * t / keplerianOrbiter.orbitalPeriod;
        assert(keplerianOrbiter.orbitalPeriod > 0);
        double E = eccentric_anomaly(M, keplerianOrbiter.eccentricity);

        Vector3 pos = keplerToCartesian(
          E,
          keplerianOrbiter.eccentricity,
          keplerianOrbiter.semiMajorAxis,
          keplerianOrbiter.inclination,
          keplerianOrbiter.longAscendingNode,
          keplerianOrbiter.longPeriapsis
        );

        //printf("%f %f %f %f %f %f %f\n", float(keplerianOrbiter.orbitalPeriod), float(t), float(E), float(M), float(pos.x), float(pos.y),  float(pos.z));

        spatial.position = pos;
      });
    }
};
