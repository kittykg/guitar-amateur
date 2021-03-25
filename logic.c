#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "logic.h"
#include "chord.h"
#include "audio.h"
#include "defs.h"

#define OFFSCREEN_BOUND(curr, res) ((curr) - (res))
#define LOWERBOUND(curr, res) ((curr) - (res) / 2)
#define UPPERBOUND(curr, res) ((curr) + (res) / 2)
#define CHORDS_INIT_CAPACITY 3
#define MEMORY_SCALING_FACTOR 2

static void pausedLogic(game_state_t* state, input_data_t* input);
static void pauseGame(game_state_t* state);
static void unpauseGame(game_state_t* state);
static void processStrum(game_state_t* state, input_data_t* input);
static chord_t* getPlayableChords(game_state_t* state, int* numOfChords);
static void checkForMissedNotes(game_state_t* state, input_data_t* input);
static void checkIfPlayerAlive(game_state_t* state);
static void checkFirstVisibleNote(game_state_t* state);
static void checkIfSongOver(game_state_t* state);
static void penalise(game_state_t* state);
static void reward(game_state_t* state);
static bool isNotePlayable(game_state_t* state, note_t* note);
static bool patternMatch(bool* keysDown, bool* chordColours);
static void updatePosition(game_state_t* state);
static long syncSectionDuration(long startPos, long endPos, int bpmTimes1000,
    int resolution);
static long getCurrentPos(game_state_t* state);
static void updateTimers(game_state_t* state);
static void processLongNote(game_state_t* state, input_data_t* input);

void logic(game_state_t* state, input_data_t* input) {
  if (state->isGamePaused) {
    pausedLogic(state, input);
    return;
  }

  if (input->pausePressed) {
    pauseGame(state);
    return;
  }

  updatePosition(state);
  updateTimers(state);

  processLongNote(state, input);

  if (input->strummed) {
    processStrum(state, input);
  }

  checkForMissedNotes(state, input);
  checkIfPlayerAlive(state);
  checkFirstVisibleNote(state);
  checkIfSongOver(state);
}

static void pausedLogic(game_state_t* state, input_data_t* input) {
  // if paused button pressed, unpause
  // if escape button pressed, set isGameOver=true;
  if (input->pausePressed) {
    unpauseGame(state);
  }

  if (input->exitPressed) {
    printf("Rage quit?\n");
    state->isGameOver = true;
  }
}

static void pauseGame(game_state_t* state) {
  pauseTimer(state->timer);
  state->isGamePaused = true;
  pauseMusic();
}

static void unpauseGame(game_state_t* state) {
  unpauseTimer(state->timer);
  state->isGamePaused = false;
  resumeMusic();
}

static void processStrum(game_state_t* state, input_data_t* input) {
  // When there are no notes left (meaning firstVisibleNote is NULL),
  // but the song is still playing then any strums will be penalised
  if (!state->firstVisibleNote) {
    penalise(state);
    return;
  }

  bool keysDown[MAX_COLOUR] = {input->greenDown,input->redDown,
    input->yellowDown, input->blueDown, input->orangeDown};

  int numOfChords;
  chord_t* chords = getPlayableChords(state, &numOfChords);
  if (!chords && !numOfChords) {
    free(chords);
    return;
  }

  assert(!!chords == !!numOfChords);

  // Pattern matching
  for (int i = 0; i < numOfChords; i++) {
    if (patternMatch(keysDown, chords[i].colours)) {
      note_status_t noteStatus;
      if ((chords[i].startNote)->length > 0) {
        state->heldChord = cloneChord(chords + i);
        noteStatus = IN_PROGRESS;
      }
      else {
        noteStatus = HIT;
      }
      for (int j = 0; j < chords[i].numOfNotes; j++) {
        reward(state);
        chords[i].startNote[j].status = noteStatus;
        state->hitTimers[chords[i].startNote[j].colour] = 200;
        state->hitCount++;
      }
      state->hitCombo++;
      if (state->hitCombo >= state->scoreMultiplier * 10
            && state->scoreMultiplier < 4) {
        state->scoreMultiplier++;
      }
      free(chords);
      return;
    }
  }
  free(chords);
}

static chord_t* getPlayableChords(game_state_t* state, int* numOfChords) {
  note_t* nextNote;
  for (nextNote = state->firstVisibleNote; nextNote->position < 
      LOWERBOUND(state->currPosition, state->currSong->resolution) ||
      nextNote->status != AVAILABLE;
      nextNote++);

  if (!isNotePlayable(state, nextNote)) {
    *numOfChords = 0;
    return NULL;
  }

  int currCap = CHORDS_INIT_CAPACITY;
  int size = 0;
  chord_t* chords = calloc(currCap, sizeof(chord_t));
  if (!chords) {
    perror("Error in allocating memory for chords");
    exit(EXIT_FAILURE);
  }

  initChord(nextNote, &chords[0]);
  size++;
  nextNote++;

  while (isNotePlayable(state, nextNote)) {
    if (nextNote->status != AVAILABLE) {
      nextNote++;
      continue;
    }

    if (nextNote->position == (chords[size - 1].startNote)->position) {
      addNoteToChord(nextNote, chords + size - 1); 
    } else {
      if (size >= currCap) {
        chord_t* newChords = realloc(chords,
            currCap * MEMORY_SCALING_FACTOR * sizeof(chord_t));
        if (!newChords) {
          perror("Error in reallocating memory for chords");
          exit(EXIT_FAILURE);
        }
        chords = newChords;
        currCap *= MEMORY_SCALING_FACTOR;
      }
      initChord(nextNote, chords + size);
      size++;
    }
    nextNote++;
  }

  *numOfChords = size;
  if (size != currCap) {
    chord_t* result = realloc(chords, size * sizeof(chord_t));
    if (!result) {
      perror("Error in reallocating memory for chords");
      exit(EXIT_FAILURE);
    }
    chords = result;
  }
  return chords;
}

static void checkForMissedNotes(game_state_t* state, input_data_t* input) {
  // check if there are any AVAILABLE notes after the playable zone
  // if there are mark all as MISSED and register miss
  if (!state->firstVisibleNote) {
    return;
  }

  for (note_t* note = state->firstVisibleNote; note->position < 
      LOWERBOUND(state->currPosition, state->currSong->resolution);
      note++) {
    if (note->status == AVAILABLE) {
      if (state->hitCombo >= MIN_COMBO) {
        state->comboBrokenTimer = 1000;
      }
      state->hitCombo = 0;
      state->scoreMultiplier = 1;
      note->status = MISS;
      penalise(state);
      playMissSound();
    }
  }
}

static void checkIfPlayerAlive(game_state_t* state) {
  // if health is equal to or below 0, set isGameOver and print
  // sad/abusive message to stdout
  if (state->health <= 0) {
    state->isGameOver = true;
    printf("You lose!\n");
  }
}

static void checkFirstVisibleNote(game_state_t* state) {
  // if state->firstVisibleNote is now offscreen, increment
  // the pointer untill it is now longer offscreen

  if (!state->firstVisibleNote) {
    return;
  }

  int bound = state->currPosition -
    state->currSong->resolution;
  while (state->firstVisibleNote->position + state->firstVisibleNote->length
      < bound) {
    if (state->firstVisibleNote + 1 >= state->currSong->notes +
        state->currSong->numOfNotes) {
      state->firstVisibleNote = NULL;
      return;
    }
    (state->firstVisibleNote)++;
  }
}


static void checkIfSongOver(game_state_t* state) {
  if (musicIsOver()) {
    printf("You've reached the end of the song!\n");
    state->isGameOver = true;
  }
}

static void penalise(game_state_t* state) {
  state->health -= PENALTY;
  if (state->health < 0) 
    state->health = 0;
}

static void reward(game_state_t* state) {
  state->score += REWARD_SCORE;
  state->health += REWARD_HEALTH;
  if (state->health > 100)
    state->health = 100;
}

static bool isNotePlayable(game_state_t* state, note_t* note) {
  return note->position <= UPPERBOUND(state->currPosition,
      state->currSong->resolution) &&
    note->position >= LOWERBOUND(state->currPosition,
        state->currSong->resolution);
}

static bool patternMatch(bool* keysDown, bool* chordColours){
  for (int i = 0; i < MAX_COLOUR; i++) {
    if (keysDown[i] != chordColours[i])
      return false;
  }
  return true;
}

static void updatePosition(game_state_t* state) {
  incrementTimer(state->timer);
  state->currPosition = getCurrentPos(state);
}

static long getCurrentPos(game_state_t* state) {
  // unit = beat duration / resolution     (A unit is a tick)

  long currentTime = state->timer->timePassed - state->currSong->offset * 1000;

  long msSinceLastSync = currentTime - state->timeOfLastSync;
  long unitsSinceLastSync 
    = state->currBPM * msSinceLastSync * state->currSong->resolution
    / 60 / 1000 / 1000;
  long currentPos = state->posOfLastSync + unitsSinceLastSync;

  // No more sync points left in the sync track
  if (!state->nextSync) {
    return currentPos;
  }

  if (currentPos < state->nextSync->position) {
    return currentPos;
  }

  // We've moved on to a new BPM
  state->timeOfLastSync += syncSectionDuration(state->posOfLastSync,
      state->nextSync->position, state->currBPM, state->currSong->resolution);
  state->posOfLastSync = state->nextSync->position;
  state->currBPM = state->nextSync->bpm;

  // Increment the next sync point
  // If there are none left we set it to null
  state->nextSync++;
  sync_t* endOfSyncs = state->currSong->syncTrack + state->currSong->numOfSync;
  if (state->nextSync >= endOfSyncs) {
    state->nextSync = NULL;
  }

  // This recursive call to the function will now use the updated posOfLastSync
  // and timeOfLastSync that we just calculated, so the currentPosition it 
  // returns will be correct.
  return getCurrentPos(state);
}

static long syncSectionDuration(long startPos, long endPos, int bpmTimes1000,
    int resolution) {
  long beatsInSyncSection = (endPos - startPos) / resolution;
  return beatsInSyncSection * 1000 * 1000 * 60 / bpmTimes1000;
}

static void updateTimers(game_state_t* state) {
  for (int i = 0; i < MAX_COLOUR; i++) {
    if (state->hitTimers[i] >= 0) {
      state->hitTimers[i] -= state->timer->delta;
      if (state->hitTimers[i] < 0) {
        state->hitTimers[i] = 0;
      }  
    }
  }
  if (state->comboBrokenTimer >= 0) {
    state->comboBrokenTimer -= state->timer->delta;
    if (state->comboBrokenTimer < 0) {
      state->comboBrokenTimer = 0;
    }  
  }
}

static void processLongNote(game_state_t* state, input_data_t* input) {
  if (state->heldChord) {
    bool keysDown[MAX_COLOUR] = {input->greenDown,input->redDown,
      input->yellowDown, input->blueDown, input->orangeDown};

    if (!patternMatch(keysDown, state->heldChord->colours)) {
      for (int i = 0; i < state->heldChord->numOfNotes; i++) {
        state->heldChord->startNote[i].status = HIT;
      }
      free(state->heldChord);
      state->heldChord = NULL;
      return;
    }
    //give points
    state->score += state->timer->delta * state->currBPM * REWARD_SCORE
      / 1000 / 1000 / 60;

    // if we've reached the end of the note, let go
    if (state->currPosition >= state->heldChord->startNote->length +
        state->heldChord->startNote->position) {
      for (int i = 0; i < state->heldChord->numOfNotes; i++) {
        state->heldChord->startNote[i].status = HIT;
      }
      free(state->heldChord);
      state->heldChord = NULL;
    }
  }
}
