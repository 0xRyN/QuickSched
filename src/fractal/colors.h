#ifndef COLORS_H
#define COLORS_H

#include <SDL.h>

SDL_Color HSL_to_RGB(float h, float s, float l);
uint32_t mandelbrot_color_gray_scale(int iter, int max_iter);
uint32_t mandelbrot_color_red_scale(int iter, int max_iter);
uint32_t mandelbrot_color_green_scale(int iter, int max_iter);
uint32_t mandelbrot_color_blue_scale(int iter, int max_iter);
uint32_t mandelbrot_color_rainbow(int iter, int max_iter);

#endif