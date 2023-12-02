#include <SDL.h>
#include "sw.h"
#include "swinit.h"

SDL_GameController *gamepad = NULL;
static int gamepad_initted = 0;

void Gamepad_Init(void)
{
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Gamepad could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        //SDL_GameController *gamepad = NULL;
        if (SDL_NumJoysticks() > 0) {
            gamepad = SDL_GameControllerOpen(0);
            printf("a gamepad is detected. \n");
        } else {
            printf("no gamepad is detected. \n");
        }
    }

    gamepad_initted = 1;
}

void Gamepad_Update(void)
{
	if (!gamepad_initted) {
		Gamepad_Init();
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





/* poll events



		switch (event.type) {
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			if(event.type == SDL_CONTROLLERBUTTONDOWN) {
				buttonPressed = 1;
			}
			switch (event.cbutton.button) {
				case SDL_CONTROLLER_BUTTON_A:
					if (buttonPressed) {
						printf("The A button is pressed\n");
						keysdown[KEY_FIRE] |= 3;
					}
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					dpad_left = buttonPressed;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					dpad_right = buttonPressed;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					dpad_up = buttonPressed;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					dpad_down = buttonPressed;
					break;
			}





jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjdddddddddddddddddddddddddddddddddddd




*/

