#include <SDL.h>
#include "gamepad.h"
#include "video.h"

SDL_GameController *gamepad = NULL;
static int gamepad_initted = 0;
static int last_input_gamepad = 0;

int btnbindings[NUM_BTNS] = {
	-1,									// BTN_UNKNOWN
	SDL_CONTROLLER_BUTTON_DPAD_LEFT,	// BTN_PULLUP
	SDL_CONTROLLER_BUTTON_DPAD_RIGHT,	// BTN_PULLDOWN
	SDL_CONTROLLER_BUTTON_Y,			// BTN_FLIP
	SDL_CONTROLLER_BUTTON_A,			// BTN_BOMB
	SDL_CONTROLLER_BUTTON_X,			// BTN_FIRE
	SDL_CONTROLLER_BUTTON_START,		// BTN_HOME
	SDL_CONTROLLER_BUTTON_LEFTSTICK,	// BTN_MISSILE
	SDL_CONTROLLER_BUTTON_RIGHTSTICK,	// BTN_STARBURST
	SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,// BTN_ACCEL
	SDL_CONTROLLER_BUTTON_LEFTSHOULDER, // BTN_DECEL
	SDL_CONTROLLER_BUTTON_BACK,			// BTN_SOUND
};

int btnsdown[NUM_BTNS];

const char *btnnames[] = {
	"A",			// 0, SDL_CONTROLLER_BUTTON_A
	"B",			// 1, SDL_CONTROLLER_BUTTON_B
	"X",			// 2, SDL_CONTROLLER_BUTTON_X
	"Y",			// 3, SDL_CONTROLLER_BUTTON_Y
	"Back",			// 4, SDL_CONTROLLER_BUTTON_BACK
	"Guide Button",	// 5, SDL_CONTROLLER_BUTTON_GUIDE
	"Start",		// 6, SDL_CONTROLLER_BUTTON_START
	"Left Stick",	// 7, SDL_CONTROLLER_BUTTON_LEFTSTICK
	"Right Stick",	// 8, SDL_CONTROLLER_BUTTON_RIGHTSTICK
	"LB",			// 9, SDL_CONTROLLER_BUTTON_LEFTSHOULDER
	"RB",			// 10, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
	"D-Pad Up",		// 11, SDL_CONTROLLER_BUTTON_DPAD_UP
	"D-Pad Down",	// 12, SDL_CONTROLLER_BUTTON_DPAD_DOWN
	"D-Pad Left",	// 13, SDL_CONTROLLER_BUTTON_DPAD_LEFT
	"D-Pad Right",	// 14, SDL_CONTROLLER_BUTTON_DPAD_RIGHT
	"Misc",			// 15, SDL_CONTROLLER_BUTTON_MISC1
	"Paddle 1",		// 16, SDL_CONTROLLER_BUTTON_PADDLE1
	"Paddle 2",		// 17, SDL_CONTROLLER_BUTTON_PADDLE2
	"Paddle 3",		// 18, SDL_CONTROLLER_BUTTON_PADDLE3
	"Paddle 4",		// 19, SDL_CONTROLLER_BUTTON_PADDLE4
	"Touchpad",		// 20, SDL_CONTROLLER_BUTTON_TOUCHPAD
};

void Gamepad_Init(void)
{
	if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
		printf("Gamepad could not initialize! SDL_Error: %s\n", SDL_GetError());
	} else {
		if (SDL_NumJoysticks() > 0) {
			gamepad = SDL_GameControllerOpen(0);
		}
	}

	gamepad_initted = 1;
}

void Gamepad_Shutdown(void)
{
	if (gamepad_initted) {
		// Close the gamepad
		if (gamepad != NULL) {
			SDL_GameControllerClose(gamepad);
			gamepad = NULL;
		}
		gamepad_initted = 0;
	}
}

static void getevents(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			last_input_gamepad = 1;
			break;
		}
	}

	if(last_input_gamepad == 1) {
		for (int i = 1; i < NUM_BTNS; ++i) {
			if (btnbindings[i] != -1) {
				if (SDL_GameControllerGetButton(gamepad, btnbindings[i])) {
					btnsdown[i] |= 3; // Button is pressed
				} else {
					btnsdown[i] &= ~1; // Button is not pressed
				}
			}
		}
	}
}

int Gamepad_GetBtn(void)
{
	int btn = -1;
	SDL_Event event;

	while (btn == -1) {
		if (SDL_WaitEventTimeout(&event, 10)) {
			if (event.type == SDL_CONTROLLERBUTTONDOWN) {
				btn = event.cbutton.button;
				break;
			} else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					return -1;
				}
			}
		}
	}
	return btn;
}

int Gamepad_GetGameBtns(void)
{
	int i, c = 0;

	getevents();
	
	if (btnsdown[BTN_FLIP]) {
		btnsdown[BTN_FLIP] = 0;
		c |= K_FLIP;
	}
	if (btnsdown[BTN_PULLUP]) {
		c |= K_FLAPU;
	}
	if (btnsdown[BTN_PULLDOWN]) {
		c |= K_FLAPD;
	}
	if (btnsdown[BTN_ACCEL]) {
		c |= K_ACCEL;
	}
	if (btnsdown[BTN_DECEL]) {
		c |= K_DEACC;
	}
	if (btnsdown[BTN_SOUND]) {
		btnsdown[BTN_SOUND] = 0;
		c |= K_SOUND;
	}
	if (btnsdown[BTN_BOMB]) {
		c |= K_BOMB;
	}
	if (btnsdown[BTN_FIRE]) {
		c |= K_SHOT;
	}
	if (btnsdown[BTN_HOME]) {
		c |= K_HOME;
	}
	if (btnsdown[BTN_MISSILE]) {
		btnsdown[BTN_MISSILE] = 0;
		c |= K_MISSILE;
	}
	if (btnsdown[BTN_STARBURST]) {
		btnsdown[BTN_STARBURST] = 0;
		c |= K_STARBURST;
	}

	// clear bits in button array
	
	for (i=0; i<NUM_BTNS; ++i) {
		btnsdown[i] &= ~2;
	}

	return c;
}

const char *Gamepad_BtnName(int btn)
{
	if (btn < 0 || btn >= sizeof(btnnames) / sizeof(btnnames[0])) {
		return "Unknown";
	}
	return btnnames[btn];
}

int isLastInputGamepad(void)
{
	return last_input_gamepad;
}

void setLastInputGamepad(int input)
{
	last_input_gamepad = input;
}

int isGamepadInitted(void) {
	return gamepad_initted;
}