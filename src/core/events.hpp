#pragma once

struct Event {};

struct QuitEvent : Event {
  bool requested = false;
};
