#include <SDL2/SDL.h>

#include "input_handler.h"

#define PAUSE_KEY  SDL_SCANCODE_P
#define STRUM_KEY  SDL_SCANCODE_K
#define EXIT_KEY   SDL_SCANCODE_ESCAPE
#define GREEN_KEY  SDL_SCANCODE_A
#define RED_KEY    SDL_SCANCODE_S
#define YELLOW_KEY SDL_SCANCODE_D
#define BLUE_KEY   SDL_SCANCODE_F
#define ORANGE_KEY SDL_SCANCODE_G

static void getEventBasedInput(input_data_t* inputData);
static void getPolledInput(input_data_t* inputData);

void getInput(input_data_t* inputData) {
  clearAllFlags(inputData);

  getEventBasedInput(inputData);
  getPolledInput(inputData);
}

static void getEventBasedInput(input_data_t* inputData) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      exit(EXIT_SUCCESS);
      break;

    case SDL_KEYDOWN:
      switch (event.key.keysym.scancode) {
      case PAUSE_KEY:
        inputData->pausePressed = true;
        break;

      case STRUM_KEY:
        inputData->strummed = true;
        break;

      case EXIT_KEY:
        inputData->exitPressed = true;
        break;

      default:
	break;
      }
    }
  }
}

static void getPolledInput(input_data_t* inputData) {
  const Uint8* state = SDL_GetKeyboardState(NULL);
  inputData->greenDown = state[GREEN_KEY];
  inputData->redDown = state[RED_KEY];
  inputData->yellowDown = state[YELLOW_KEY];
  inputData->blueDown = state[BLUE_KEY];
  inputData->orangeDown = state[ORANGE_KEY];
}
