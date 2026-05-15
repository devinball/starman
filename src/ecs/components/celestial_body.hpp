// use multipole expansion for more accuracy, the
// number of terms can be adjusted as an object is
// less spherical
// https://en.wikipedia.org/wiki/Multipole_expansion

// atmospheric model: https://en.wikipedia.org/wiki/NRLMSISE-00
// probably don't need that complex tho

struct CelestialBody {
  float mass;
};
