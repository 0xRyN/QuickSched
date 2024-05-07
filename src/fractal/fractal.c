#include "fractal.h"

int mandelbrot(Complex c, int max_iter) {
    Complex z = {0};
    int n = 0;
    while (z.real * z.real + z.imag * z.imag <= 4 && n < max_iter) {
        double temp = z.real * z.real - z.imag * z.imag + c.real;
        z.imag = 2 * z.real * z.imag + c.imag;
        z.real = temp;
        n++;
    }
    return n;
}
