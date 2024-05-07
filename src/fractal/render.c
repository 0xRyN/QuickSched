#define _POSIX_C_SOURCE 199309L

#include "render.h"

#include <assert.h>
#include <time.h>

#include "../sched.h"
#include "colors.h"
#include "fractal.h"
#include "fractal_args.h"
#include "fractal_task.h"

double offsetX = 0.0;
double offsetY = 0.0;
double scale = 1.0;

struct scheduler *s;
uint32_t *pixels;

void init_scheduler(SDL_Renderer *renderer, int width, int height,
                    int max_iter) {
    SDL_Texture *texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_STATIC, width, height);

    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));

    if (!pixels) {
        perror("Memory allocation failed for pixels");
        exit(EXIT_FAILURE);
    }

    fractal_args *args =
        new_fractal_args(renderer, texture, pixels, 0, width * height, width,
                         height, offsetX, offsetY, scale, max_iter);

    s = stealing_sched_init(0, 1000, compute_fractal, args);
}

void init_renderer(int width, int height) {
    pixels = malloc(width * height * sizeof(uint32_t));
    if (!pixels) {
        perror("Memory allocation failed for pixels");
        exit(EXIT_FAILURE);
    }
}

void cleanup_renderer() { free(pixels); }

void render_mandelbrot(SDL_Renderer *renderer, int width, int height,
                       int max_iter) {
    SDL_Texture *texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_STATIC, width, height);
    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));

    double ratioX = 1.0, ratioY = 1.0;

    if (width > height) {
        ratioX = (double)width / height;
        ratioY = 1;
    } else {
        ratioX = 1;
        ratioY = (double)height / width;
    }

    struct timespec begin, end;
    double delay;
    clock_gettime(CLOCK_MONOTONIC, &begin);

    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            double zx =
                ((ratioX * -1) + (px + offsetX) * (ratioX * 2.0 / width)) /
                scale;
            double zy =
                ((ratioY * -1) + (py + offsetY) * (ratioY * 2.0 / height)) /
                scale;

            Complex c = {.real = zx, .imag = zy};
            int n = mandelbrot(c, max_iter);
            pixels[py * width + px] = mandelbrot_color_red_scale(n, max_iter);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    delay = end.tv_sec + end.tv_nsec / 1000000000.0 -
            (begin.tv_sec + begin.tv_nsec / 1000000000.0);
    printf("Frametime: %lf seconds.\n", delay);

    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);
    free(pixels);
}

void render_mandelbrot_parallel(SDL_Renderer *renderer, int width, int height,
                                int max_iter) {
    SDL_Texture *texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_STATIC, width, height);

    fractal_args *args =
        new_fractal_args(renderer, texture, pixels, 0, width * height, width,
                         height, offsetX, offsetY, scale, max_iter);
    int rc = sched_spawn(compute_fractal, args, s);
    assert(rc >= 0);

    struct timespec begin, end;
    double delay;
    clock_gettime(CLOCK_MONOTONIC, &begin);
    wait_for_threads(s);
    clock_gettime(CLOCK_MONOTONIC, &end);

    delay = end.tv_sec + end.tv_nsec / 1000000000.0 -
            (begin.tv_sec + begin.tv_nsec / 1000000000.0);
    printf("Frametime: %lf seconds.\n", delay);

    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);
}
