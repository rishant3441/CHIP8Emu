#include <iostream>
#include <SDL.h>
#include "Chip8Emu.h"

int main(int argc, char** argv) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    window = SDL_CreateWindow("Chip 8 EMU", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1216, 608, SDL_WINDOW_SHOWN);

    if (!window)
    {
        std::cout << "Error creating window: " << SDL_GetError() << std::endl;
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Chip8Emu emulator(1216, 608, "snake.ch8");

    bool keys[SDL_NUM_SCANCODES] = {false};
    bool running = true;
    const double targetFrameTime = 1.0 / 60.0;
    double freq = (double)SDL_GetPerformanceFrequency();
    SDL_Event e;
    while (running)
    {
        Uint64 frameStart = SDL_GetPerformanceCounter();

        while (SDL_PollEvent(&e) != 0)
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    keys[e.key.keysym.scancode] = true;
                    break;
                case SDL_KEYUP:
                    keys[e.key.keysym.scancode] = false;
                    break;
            }
        }

        if (!running) break;

        emulator.handle_input(keys);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        emulator.loop(renderer);
        SDL_RenderPresent(renderer);

        Uint64 frameEnd = SDL_GetPerformanceCounter();
        double seconds = (frameEnd - frameStart) / freq;

        double delaySeconds = targetFrameTime - seconds;

        if (delaySeconds > 0)
        {
            Uint32 delayMS = (Uint32)(delaySeconds * 1000.0);
            if (delayMS > 0)
            {
                SDL_Delay(delayMS);
            }

            while (((SDL_GetPerformanceCounter() - frameStart) / freq) < targetFrameTime)
            {
            }
        }

        double frameTime = ((SDL_GetPerformanceCounter() - frameStart) / freq);
        //std::cout << "fps: " << (1.0 / frameTime) << std::endl;
    }
    emulator.exit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
