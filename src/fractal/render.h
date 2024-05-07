#ifndef RENDER_H
#define RENDER_H

#include <SDL.h>

void init_scheduler(SDL_Renderer *renderer, int width, int height,
                    int max_iter);

void init_renderer(int width, int height);
void cleanup_renderer();

void render_mandelbrot(SDL_Renderer *renderer, int width, int height,
                       int max_iter);

void render_mandelbrot_parallel(SDL_Renderer *renderer, int width, int height,
                                int max_iter);
#endif
