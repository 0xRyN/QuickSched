#include "colors.h"

SDL_Color HSL_to_RGB(float h, float s, float l) {
    float c = (1 - fabs(2 * l - 1)) * s;
    float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    float m = l - c / 2;
    float r = 0, g = 0, b = 0;

    if (h >= 0 && h < 60) {
        r = c, g = x, b = 0;
    } else if (h >= 60 && h < 120) {
        r = x, g = c, b = 0;
    } else if (h >= 120 && h < 180) {
        r = 0, g = c, b = x;
    } else if (h >= 180 && h < 240) {
        r = 0, g = x, b = c;
    } else if (h >= 240 && h < 300) {
        r = x, g = 0, b = c;
    } else if (h >= 300 && h < 360) {
        r = c, g = 0, b = x;
    }

    SDL_Color rgb = {(uint8_t)((r + m) * 255), (uint8_t)((g + m) * 255),
                     (uint8_t)((b + m) * 255)};
    return rgb;
}

uint32_t mandelbrot_color_gray_scale(int iter, int max_iter) {
    if (iter == max_iter)
        return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), 0, 0, 0);
    int c = 255 * iter / max_iter;
    return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), c, c, c);
}

uint32_t mandelbrot_color_red_scale(int iter, int max_iter) {
    if (iter == max_iter)
        return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), 0, 0, 0);
    int c = 255 * iter / max_iter;
    return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), c, 0, 0);
}

uint32_t mandelbrot_color_green_scale(int iter, int max_iter) {
    if (iter == max_iter)
        return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), 0, 0, 0);
    int c = 255 * iter / max_iter;
    return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), 0, c, 0);
}

uint32_t mandelbrot_color_blue_scale(int iter, int max_iter) {
    if (iter == max_iter)
        return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), 0, 0, 0);
    int c = 255 * iter / max_iter;
    return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), 0, 0, c);
}

uint32_t mandelbrot_color_rainbow(int iter, int max_iter) {
    if (iter == max_iter)
        return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), 0, 0, 0);
    float hue = 360.0f * iter / max_iter;
    SDL_Color color = HSL_to_RGB(hue, 1.0, 0.5);
    return SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888), color.r,
                      color.g, color.b);
}
