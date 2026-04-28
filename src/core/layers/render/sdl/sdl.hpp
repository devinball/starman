/*#pragma once

#include <SDL3/SDL.h>

struct SDLContext {
  SDL_Window* window;
  SDL_GLContext gl;
  SDL_Surface surface;
};

SDLContext createSDLContext() {
  SDLContext context;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("Error initializing SDL: %s\n", SDL_GetError());
  }

  context.window = SDL_CreateWindow("title", 500, 500, SDL_WINDOW_VULKAN);
  context.gl = SDL_GL_CreateContext(context.window);

  printf("Started SDL...\n");

  return context;
}

void destroySDLContext(SDLContext& ctx) {

}

bool pollSDLEvents(SDLContext& ctx) {
  SDL_Event event;

  bool quit = false;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
      quit = true;
    }
  }

  return quit;
}
*/