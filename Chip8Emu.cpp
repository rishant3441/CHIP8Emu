//
// Created by rishant3441 on 3/5/2025.
//

#include <iostream>
#include <fstream>
#include "Chip8Emu.h"

#undef SETVX
#undef INCR_I
#undef JUMP_WITH_OFFSET

Chip8Emu::Chip8Emu(int width, int height, std::string program) {
    pixel_size = width / 64;

    init(program);
}

void Chip8Emu::init(std::string program) {
    buffer = new unsigned char[4096];
    if (buffer == nullptr)
    {
        std::cerr << "Memory alloc failed!" << std::endl;
        return;
    }

    unsigned char fontset[80] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < 80; i++)
    {
        buffer[0x050 + i] = fontset[i];
    }

    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            display[i][j] = true;
        }
    }

    stack = new unsigned short[256];
    memset(stack, 0, 16 * sizeof(unsigned short));

    PC = 0x200;
    SP = 0;
    I = 0;
    v = new uint8_t[16];
    memset(v, 0, 16);

    const char* filename = program.c_str();
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file) {
        std::cerr << "Error opening file!\n";
        return;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Define the start index and compute the required size
    const size_t startIndex = 0x200;
    size_t numShorts = fileSize / sizeof(uint16_t);
    size_t totalSize = startIndex + numShorts;

    // Read the binary file into the array starting at index 0x200
    file.read(reinterpret_cast<char*>(&buffer[startIndex]), fileSize);
    file.close();

    std::cout << "File read successfully into array starting at index 0x200.\n";
}

void Chip8Emu::exit() {
    delete[] stack;
    delete[] buffer;
    delete[] v;
}

void Chip8Emu::loop(SDL_Renderer* renderer) {
    for (int i = 0; i < 11; i++)
        process_instruction();

    // timers
    if (delay_timer > 0)
    {
        delay_timer--;
    }
    if (sound_timer > 0)
    {
        // play sound here too
        sound_timer--;
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            if (display[y][x])
            {
                SDL_Rect rect = {x * pixel_size, y * pixel_size, pixel_size, pixel_size};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void Chip8Emu::handle_input(bool* new_keys) {
    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        keys[i] = new_keys[i];
    }
}

void Chip8Emu::process_instruction() {
    // fetch
    auto curr_inst = (unsigned short) ((buffer[PC] << 8) | buffer[PC+1]);
    PC += 2;

    // decode + execute step here
    unsigned short first_nibble = (curr_inst & 0xF000) >> 12;
    unsigned short X = (curr_inst & 0x0F00) >> 8;
    unsigned short Y = (curr_inst & 0x00F0) >> 4;
    unsigned short N = curr_inst & 0x000F;
    unsigned short NN = curr_inst & 0x00FF;
    unsigned short NNN = curr_inst & 0x0FFF;

    switch (first_nibble) {
        case 0x0:
            if (curr_inst == 0x00E0)
            {
                memset(display, 0, sizeof(display));
            }
            else if (curr_inst == 0x00EE)
            {
                PC = stack[SP];
                SP--;
            }
            else
            {
                std::cout << "SYS Addr ran, not implement" << std::endl;
            }
            break;
        case 0x1:
            PC = NNN;
            break;
        case 0x2:
            SP++;
            stack[SP] = PC;
            PC = NNN;
            break;
        case 0x3:
            if (v[X] == NN)
            {
                PC+=2;
            }
            break;
        case 0x4:
            if (v[X] != NN)
            {
                PC+=2;
            }
            break;
        case 0x5:
            if (v[X] == v[Y])
            {
                PC+=2;
            }
            break;
        case 0x6:
            v[X] = NN;
            break;
        case 0x7:
            v[X] += NN;
            break;
        case 0x8:
            switch (N)
            {
                case 0x0:
                    v[X] = v[Y];
                    break;
                case 0x1:
                    v[X] = v[X] | v[Y];
                    v[0xF] = 0;
                    break;
                case 0x2:
                    v[X] = v[X] & v[Y];
                    v[0xF] = 0;
                    break;
                case 0x3:
                    v[X] = v[X] ^ v[Y];
                    v[0xF] = 0;
                    break;
                case 0x4: {
                    unsigned short sum = v[X] + v[Y];
                    v[X] = sum & 0xFF;
                    if (sum > 255)
                    {
                        v[0xF] = 1;
                    }
                    else
                    {
                        v[0xF] = 0;
                    }
                    break;
                }
                case 0x5:
                {
                    unsigned short temp = v[X] - v[Y];
                    if (v[X] >= v[Y])
                    {
                        v[X] = temp;
                        v[0xF] = 1;
                    }
                    else
                    {
                        v[X] = temp;
                        v[0xF] = 0;
                    }
                    break;
                }
                case 0x6: // ambiguous
                {
                    unsigned short temp = v[X];
#ifdef SETVX
                    v[X] = v[Y];
#endif
                    v[X] = v[X] >> 1;
                    v[0xF] = temp & 1;
                    break;
                }
                case 0x7:
                {
                    unsigned short temp = v[Y] - v[X];
                    if (v[Y] >= v[X])
                    {
                        v[X] = temp;
                        v[0xF] = 1;
                    }
                    else
                    {
                        v[X] = temp;
                        v[0xF] = 0;
                    }
                }
                    break;
                case 0xE: // ambiguous
                {
                    unsigned short temp = v[X];
#ifdef SETVX
                    v[X] = v[Y];
#endif
                    v[X] = v[X] << 1;
                    v[0xF] = (temp & 0x80) >> 7;
                    break;
                }
            }
            break;
        case 0x9:
            if (v[X] != v[Y])
            {
                PC+=2;
            }
            break;
        case 0xA:
            I = NNN;
            break;
        case 0xB:
            PC = NNN + v[0];
#ifdef JUMP_WITH_OFFSET
            PC = NNN + v[X];
#endif
            break;
        case 0xC:
            v[X] = (rand() & 0xFF) & NN;
            break;
        case 0xD:
        {
            unsigned short x = v[X] % 64;
            unsigned short y = v[Y] % 32;
            unsigned short height = N;
            v[0xF] = 0;

            for (int row = 0; row < height; row++) {
                unsigned char pixel = buffer[I + row];
                for (int col = 0; col < 8; col++) {
                    if ((pixel & (0x80 >> col)) != 0) {
                        int x_coord = (x + col) % 64;
                        int y_coord = (y + row) % 32;
                        if (display[y_coord][x_coord]) {
                            v[0xF] = 1;
                        }
                        display[y_coord][x_coord] ^= 1;
                    }
                }
            }
        }
            break;
        case 0xE:
            if (NN == 0x9E)
            {
                if (translate(v[X]))
                {
                    PC+=2;
                }
            }
            else if (NN == 0xA1)
            {
                if (!translate(v[X]))
                {
                    PC+=2;
                }
            }
            break;
        case 0xF:
            if (NN == 0x07)
            {
                v[X] = delay_timer;
            }
            else if (NN == 0x0A)
            {
                bool foundKey = false;
                SDL_Event event;
                const double targetFrameTime = 1.0 / 60.0;
                double freq = (double)SDL_GetPerformanceFrequency();
                while (!foundKey)
                {
                    Uint64 frameStart = SDL_GetPerformanceFrequency();
                    while (SDL_PollEvent(&event) != 0)
                    {
                        switch (event.type)
                        {
                            case SDL_KEYDOWN:
                                if (translate(event.key.keysym.scancode) != 255)
                                {
                                    foundKey = true;
                                    v[X] = translate(event.key.keysym.scancode);
                                    break;
                                }
                        }
                        if (foundKey) break;
                    }

                    if (delay_timer > 0)
                    {
                        delay_timer--;
                    }
                    if (sound_timer > 0)
                    {
                        // play sound here too
                        sound_timer--;
                    }

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
                }
            }
            else if (NN == 0x15)
            {
                delay_timer = v[X];
            }
            else if (NN == 0x18)
            {
                sound_timer = v[X];
            }
            else if (NN == 0x1E)
            {
                I = I + v[X];
            }
            else if (NN == 0x29)
            {
                I = 0x050 + (v[X] * 5);
            }
            else if (NN == 0x33)
            {
                buffer[I] = v[X] / 100;
                buffer[I+1] = (v[X] / 10) % 10;
                buffer[I+2] = (v[X]) % 10;
            }
            else if (NN == 0x55)
            {
                for (int i = 0; i <= X; i++)
                {
#ifdef INCR_I
                    buffer[I] = v[i];
                    I++;
#else
                    buffer[I+i] = v[i];
#endif
                }
            }
            else if (NN == 0x65)
            {
                for (int i = 0; i <= X; i++)
                {
                    v[i] = buffer[I + i];
#ifdef INCR_I
                    v[i] = buffer[I];
                    I++;
#else
                    v[i] = buffer[I+i];
#endif
                }
            }
            break;
        default:
            std::cout << "case not handled" << std::endl;
    }
}

bool Chip8Emu::translate(uint8_t vx) {
    switch (vx)
    {
        case 0x0:
            return keys[SDL_GetScancodeFromKey(SDLK_x)];
            break;
        case 0x1:
            return keys[SDL_GetScancodeFromKey(SDLK_1)];
            break;
        case 0x2:
            return keys[SDL_GetScancodeFromKey(SDLK_2)];
            break;
        case 0x3:
            return keys[SDL_GetScancodeFromKey(SDLK_3)];
            break;
        case 0x4:
            return keys[SDL_GetScancodeFromKey(SDLK_q)];
            break;
        case 0x5:
            return keys[SDL_GetScancodeFromKey(SDLK_w)];
            break;
        case 0x6:
            return keys[SDL_GetScancodeFromKey(SDLK_e)];
            break;
        case 0x7:
            return keys[SDL_GetScancodeFromKey(SDLK_a)];
            break;
        case 0x8:
            return keys[SDL_GetScancodeFromKey(SDLK_s)];
            break;
        case 0x9:
            return keys[SDL_GetScancodeFromKey(SDLK_d)];
            break;
        case 0xA:
            return keys[SDL_GetScancodeFromKey(SDLK_z)];
            break;
        case 0xB:
            return keys[SDL_GetScancodeFromKey(SDLK_c)];
            break;
        case 0xC:
            return keys[SDL_GetScancodeFromKey(SDLK_4)];
            break;
        case 0xD:
            return keys[SDL_GetScancodeFromKey(SDLK_r)];
            break;
        case 0xE:
            return keys[SDL_GetScancodeFromKey(SDLK_f)];
            break;
        case 0xF:
            return keys[SDL_GetScancodeFromKey(SDLK_v)];
            break;
    }
}

uint8_t Chip8Emu::translate(SDL_Scancode keydown)
{
    switch (keydown)
    {
        case SDL_SCANCODE_1:
            return 1;
        case SDL_SCANCODE_2:
            return 2;
        case SDL_SCANCODE_3:
            return 3;
        case SDL_SCANCODE_4:
            return 0xC;
        case SDL_SCANCODE_Q:
            return 4;
        case SDL_SCANCODE_W:
            return 5;
        case SDL_SCANCODE_E:
            return 6;
        case SDL_SCANCODE_R:
            return 0xD;
        case SDL_SCANCODE_A:
            return 0x7;
        case SDL_SCANCODE_S:
            return 0x8;
        case SDL_SCANCODE_D:
            return 0x9;
        case SDL_SCANCODE_F:
            return 0xE;
        case SDL_SCANCODE_Z:
            return 0xA;
        case SDL_SCANCODE_X:
            return 0x0;
        case SDL_SCANCODE_C:
            return 0xB;
        case SDL_SCANCODE_V:
            return 0xF;
        default:
            return 255;
    }

}

int get_nibble(int val, int bits, int val_to_binary_and = 0xFFFF) //extracts 4 bits from val
{
    return ((val & val_to_binary_and) >> bits);
}
