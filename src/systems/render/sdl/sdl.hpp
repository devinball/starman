#pragma once

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

void renderModel() {
  // draw a model at a specific location
}

void renderMesh() {
  // draws a mesh, default with wireframe, can specify material/shader
  // assumes pre-rotated mesh?
}

void renderTexture() {
  // draw texture with parameters x, y, width, height, texture
}

void renderShader() {
  // treat the entire screen as output

}

void clear() {

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
