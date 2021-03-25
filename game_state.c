#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "game_state.h"
#include "song.h"
#include "ga_timer.h"
#include "note.h"

game_state_t* initGameState(song_t* song){
  game_state_t* state = malloc(sizeof(game_state_t));
  if (!state) {
    perror("Error in initialising game state");
    exit(EXIT_FAILURE);
  }
  state->currSong = song;
  state->score = 0;
  state->health = MAX_HEALTH;
  state->firstVisibleNote = song->notes;
  state->isGamePaused = false;
  state->isGameOver = false;
  state->timer = initTimer();
  state->currPosition = 0;
  state->timeOfLastSync = 0;
  state->posOfLastSync = 0;
  state->currBPM = song->syncTrack->bpm;
  if (song->numOfSync == 0) {
    state->nextSync = NULL;
  } else {
    state->nextSync = song->syncTrack + 1;
  }
  memset(state->hitTimers, 0, MAX_COLOUR * sizeof(long));
  state->heldChord = NULL;
  state->hitCount = 0;
  state->hitCombo = 0;
  state->comboBrokenTimer = 0;
  state->scoreMultiplier = 1;
  return state;
}

void cleanupGameState(game_state_t* gameState){
  cleanupSong(gameState->currSong);
  cleanupTimer(gameState->timer);
  free(gameState->heldChord);
  free(gameState);
}
