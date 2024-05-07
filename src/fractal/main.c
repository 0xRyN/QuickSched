#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>

#include "../sched.h"
#include "render.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

#define MOVE_STEP 10
#define ZOOM_STEP 0.1

extern double offsetX;
extern double offsetY;
extern double scale;

void handle_keydown(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT:
            offsetX -= MOVE_STEP;
            break;

        case SDLK_RIGHT:
            offsetX += MOVE_STEP;
            break;

        case SDLK_UP:
            offsetY -= MOVE_STEP;
            break;

        case SDLK_DOWN:
            offsetY += MOVE_STEP;
            break;

        case SDLK_PLUS:
        case SDLK_KP_PLUS:
            scale += ZOOM_STEP;
            break;

        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            scale -= ZOOM_STEP;
            break;

        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n",
                SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Fractal Viewer", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                              WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n",
                SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n",
                SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool quit = false;
    SDL_Event e;

    init_scheduler(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, 300);
    init_renderer(WINDOW_WIDTH, WINDOW_HEIGHT);

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN) {
                handle_keydown(e.key.keysym.sym);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_mandelbrot_parallel(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, 300);

        SDL_RenderPresent(renderer);
    }

    cleanup_renderer();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
