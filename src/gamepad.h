//
// Gamepad Interface
//

#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

#include <SDL2/SDL.h>

void Gamepad_Init(void);
void Gamepad_Update(SDL_Event event);
void Gamepad_CheckState(void);

#endif