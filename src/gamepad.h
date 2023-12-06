//
// Gamepad Interface
//

#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

typedef enum {
	BTN_UNKNOWN,
	BTN_PULLUP,
	BTN_PULLDOWN,
	BTN_FLIP,
	BTN_BOMB,
	BTN_FIRE,
	BTN_HOME,
	BTN_MISSILE,
	BTN_STARBURST,
	BTN_ACCEL,
	BTN_DECEL,
	BTN_SOUND,
	NUM_BTNS,
} sopbtn_t;

void Gamepad_Init(void);
int Gamepad_GetBtn(void);
int Gamepad_GetGameBtns(void);
const char *Gamepad_BtnName(int btn);

int isLastInputGamepad(void);
void setLastInputGamepad(int input);
int isGamepadInitted(void);

extern int btnbindings[NUM_BTNS];
extern int btnsdown[NUM_BTNS];

#endif
