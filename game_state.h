#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>

#include "note.h"
#include "song.h"
#include "ga_timer.h"
#include "chord.h"

#define MAX_HEALTH 100

typedef struct {
  song_t* currSong;
  int score;
  int health;
  note_t* firstVisibleNote;
  bool isGamePaused;
  bool isGameOver;
  ga_timer_t* timer;
  long currPosition;
  long timeOfLastSync;
  long posOfLastSync;
  long currBPM;
  sync_t* nextSync;
  long hitTimers[MAX_COLOUR];
  chord_t* heldChord;
  int hitCount;
  int hitCombo;
  long comboBrokenTimer;
  int scoreMultiplier;
} game_state_t;

game_state_t* initGameState(song_t* song);
void cleanupGameState(game_state_t* gameState);

#endif // GAME_STATE_H
