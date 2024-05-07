#ifndef FRACTAL_ARGS_H
#define FRACTAL_ARGS_H

#include <SDL.h>

#include "fractal.h"

typedef struct fractal_args {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *pixels;
    int start, end;
    int width, height;
    double offsetX, offsetY, scale;
    int max_iter;
} fractal_args;

fractal_args *new_fractal_args(SDL_Renderer *renderer, SDL_Texture *texture,
                               uint32_t *pixels, int start, int end, int width,
                               int height, double offsetX, double offsetY,
                               double scale, int max_iter);

#endif