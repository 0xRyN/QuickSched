#ifndef FRACTAL_H
#define FRACTAL_H

typedef struct {
    double real;
    double imag;
} Complex;

int mandelbrot(Complex c, int max_iter);

#endif
