#include "fractal_args.h"

#include <stdlib.h>

fractal_args *new_fractal_args(SDL_Renderer *renderer, SDL_Texture *texture,
                               uint32_t *pixels, int start, int end, int width,
                               int height, double offsetX, double offsetY,
                               double scale, int max_iter) {
    fractal_args *args = malloc(sizeof(fractal_args));
    if (args == NULL) return NULL;

    args->renderer = renderer;
    args->texture = texture;
    args->pixels = pixels;
    args->start = start;
    args->end = end;
    args->width = width;
    args->height = height;
    args->offsetX = offsetX;
    args->offsetY = offsetY;
    args->scale = scale;
    args->max_iter = max_iter;

    return args;
}
