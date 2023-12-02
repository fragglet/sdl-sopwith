//
// Gamepad Interface
//

#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

#include "sw.h"

void Gamepad_Init(void);
void Gamepad_Update(void);
void Gamepad_Shutdown(void);

int buttonPressed;
int dpad_left;
int dpad_right;
int dpad_up;
int dpad_down;

#endif
