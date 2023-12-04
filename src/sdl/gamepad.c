#include <SDL.h>
#include "video.h"

SDL_GameController *gamepad = NULL;
static int gamepad_initted = 0;

int btnbindings[NUM_KEYS] = {
	-1,                    				// KEY_UNKNOWN
	SDL_CONTROLLER_BUTTON_DPAD_LEFT,	// KEY_PULLUP
	SDL_CONTROLLER_BUTTON_DPAD_RIGHT,   // KEY_PULLDOWN
	SDL_CONTROLLER_BUTTON_Y,  			// KEY_FLIP
	SDL_CONTROLLER_BUTTON_A,			// KEY_BOMB
	SDL_CONTROLLER_BUTTON_X,			// KEY_FIRE
	SDL_CONTROLLER_BUTTON_START,		// KEY_HOME
	SDL_CONTROLLER_BUTTON_LEFTSTICK,	// KEY_MISSILE
	SDL_CONTROLLER_BUTTON_RIGHTSTICK,	// KEY_STARBURST
	SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,// KEY_ACCEL
	SDL_CONTROLLER_BUTTON_LEFTSHOULDER, // KEY_DECEL
	SDL_CONTROLLER_BUTTON_BACK,			// KEY_SOUND
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

void Gamepad_Update(void) {
    if (!gamepad_initted || gamepad == NULL) {
        Gamepad_Init();
        return;
    }

    for (int i = 1; i < NUM_KEYS; ++i) {
        if (btnbindings[i] != -1) {
            if (SDL_GameControllerGetButton(gamepad, btnbindings[i])) {
                keysdown[i] |= 3; // Button is pressed
            } else {
                keysdown[i] &= ~1; // Button is not pressed
            }
        }
    }
}

void Gamepad_Shutdown(void)
{
	if (!gamepad_initted) {
		return;
	}
    // Close the gamepad
    if (gamepad != NULL) {
        SDL_GameControllerClose(gamepad);
        gamepad = NULL;
    }

	gamepad_initted = 0;
}