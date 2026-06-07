#pragma once

#include <vector>
#include <stdint.h>

struct InputEvent {
  uint64_t key;
};

struct InputBuffer {
  private:
    std::vector<InputEvent> events;
  public:
    // of course this should be done right in the future
    bool forward;
    bool backward;
    bool left;
    bool right;
    bool up;
    bool down;

    // mouse
    double dx;
    double dy;



    std::vector<InputEvent> getEvents() const { return events; }
};
