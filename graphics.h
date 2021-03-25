#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>

#include "input_data.h"
#include "game_state.h"

void render(game_state_t* state, input_data_t* input);
void initGraphics(void);
void cleanupGraphics(void);

#endif // GRAPHICS_H
