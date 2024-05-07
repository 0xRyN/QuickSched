#include "fractal_task.h"

#include <assert.h>

#include "colors.h"
#include "fractal.h"
#include "fractal_args.h"

void compute_fractal_section(void *closure) {
    fractal_args *args = (fractal_args *)closure;
    int width = args->width, height = args->height;
    double ratioX = 1.0, ratioY = 1.0;

    if (width > height) {
        ratioX = (double)width / height;
    } else {
        ratioY = (double)height / width;
    }

    for (int i = args->start; i < args->end; i++) {
        int y = i / width;
        int x = i % width;
        double zx =
            ((ratioX * -1) + (x + args->offsetX) * (ratioX * 2.0 / width)) /
            args->scale;
        double zy =
            ((ratioY * -1) + (y + args->offsetY) * (ratioY * 2.0 / height)) /
            args->scale;
        Complex z = {zx, zy};
        int iter = mandelbrot(z, args->max_iter);
        args->pixels[y * width + x] =
            mandelbrot_color_red_scale(iter, args->max_iter);
    }

    free(args);
}

void compute_fractal(void *closure, struct scheduler *s) {
    fractal_args *args = (fractal_args *)closure;
    int width = args->width;
    int num_pixels = args->end - args->start;
    int threshold = width * width / 64;

    if (num_pixels <= threshold) {
        compute_fractal_section(args);
    } else {
        int mid = args->start + num_pixels / 2;
        fractal_args *args1 = new_fractal_args(
            args->renderer, args->texture, args->pixels, args->start, mid,
            width, args->height, args->offsetX, args->offsetY, args->scale,
            args->max_iter);
        fractal_args *args2 =
            new_fractal_args(args->renderer, args->texture, args->pixels, mid,
                             args->end, width, args->height, args->offsetX,
                             args->offsetY, args->scale, args->max_iter);

        int rc = sched_spawn(compute_fractal, args1, s);
        assert(rc >= 0);
        rc = sched_spawn(compute_fractal, args2, s);
        assert(rc >= 0);
    }
}