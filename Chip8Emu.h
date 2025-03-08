//
// Created by rishant3441 on 3/5/2025.
//

#ifndef CHIP8EMU_CHIP8EMU_H
#define CHIP8EMU_CHIP8EMU_H


#include <SDL.h>

class Chip8Emu {
public:
    Chip8Emu(int width, int height, std::string program);

    void init(std::string program);
    void exit();
    void loop(SDL_Renderer* renderer);
    void handle_input(bool new_keys[]);

    int pixel_size;

    unsigned char* buffer;
    unsigned short PC;
    unsigned short I;
    unsigned short* stack;
    unsigned short SP;
    uint8_t* v;
    uint8_t delay_timer;
    uint8_t sound_timer;

    bool display[32][64];
    bool keys[SDL_NUM_SCANCODES] = {false};

private:
    void process_instruction();
    bool translate(uint8_t vx);
    uint8_t translate(SDL_Scancode);
};


#endif //CHIP8EMU_CHIP8EMU_H
