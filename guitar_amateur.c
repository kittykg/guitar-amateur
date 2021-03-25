#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "audio.h"
#include "defs.h"
#include "file_parser.h"
#include "song.h"
#include "game_state.h"
#include "input_data.h"
#include "input_handler.h"
#include "logic.h"
#include "graphics.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BUFFER_SIZE 50

static void init(game_state_t** state, int argc, char** argv, input_data_t** input);
static void initSDL(void);
static song_t* getSong(int argc, char** argv);

static void printResults(game_state_t* state);

static void cleanup(void);
static void cleanupSDL(void);

static game_state_t* state;
static input_data_t* input;

int main(int argc, char** argv) {
  atexit(cleanup);

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s <song-dir-path> "
      "<difficulty: Easy/Medium/Hard/Expert>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  init(&state, argc, argv, &input);

  playMusic();
  startTimer(state->timer);

  while (!(state->isGameOver)) {
    getInput(input);
    logic(state, input);
    render(state, input);
  }
  printResults(state);

  return EXIT_SUCCESS;
}

static void init(game_state_t** state, int argc, char** argv, input_data_t** input) {
  initSDL();
  initAudio(argv[1]);

  song_t* currSong = getSong(argc, argv);
  *state = initGameState(currSong);
  *input = initInputData();
  initGraphics();
}

static void initSDL(void) {
  if (SDL_Init(0)) {
    fprintf(stderr, "Could not initialize SDL due to: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
}

static song_t* getSong(int argc, char** argv) {
  char chartName[] = CHART_FILENAME;
  int size = strlen(argv[1]) + strlen(chartName) + 1;
  char songFilePath[size];

  strcpy(songFilePath, argv[1]);
  strcat(songFilePath, chartName);

  FILE* chart = fopen(songFilePath, "r");
  if (!chart) {
    perror("Could not open song file");
    exit(EXIT_FAILURE);
  }
  song_t* song;
  if (argc == 2) { // default difficulty
    song = parseSongFile(chart, "[]");
  } else {
    char difficulty[BUFFER_SIZE];
    sprintf(difficulty, "[%c%sSingle]",toupper(argv[2][0]), argv[2] + 1);
    for (int i = 2; i <= strlen(argv[2]); i++){
      difficulty[i] = tolower(difficulty[i]);
    }
    song = parseSongFile(chart, difficulty);
  }
  fclose(chart);
  return song;
}

static void printResults(game_state_t* state) {
  printf("You score %d on %s\n", state->score, state->currSong->name);
  int numOfNotesPassed;
  if (!state->firstVisibleNote) {
    numOfNotesPassed = state->currSong->numOfNotes;
  } else {
    numOfNotesPassed = state->firstVisibleNote - state->currSong->notes;
  }
  if (numOfNotesPassed == 0) {
    printf("Hit precentage: 100%%\n");
  } else {
    printf("Hit precentage: %.2f%%\n", 100 * ((double) state->hitCount/
          numOfNotesPassed));
  }
}

static void cleanup(void) {
  cleanupGraphics();
  cleanupAudio();
  if (state) {
    cleanupGameState(state);
  }
  cleanupSDL();
}

static void cleanupSDL(void) {
  SDL_Quit();
}
