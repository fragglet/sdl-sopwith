//
// Gamepad Interface
//

#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

void Gamepad_Init(void);
void Gamepad_Update(void);
void Gamepad_CheckState(void);

extern int btnbindings[NUM_KEYS];

#endif
