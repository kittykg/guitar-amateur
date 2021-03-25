#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "file_parser.h"

#define INIT_SIZE 1024

static FILE* file;
static int numOfNotes = 0;
static int numOfSync = 0;
static long res;
static double offset;
static int notesCurrCap = INIT_SIZE;
static int syncCurrCap = INIT_SIZE;

static void* expandArray(void* array, int* currCap, int elemSize);
static char* getName(char* target);
static void getSyncTrack(sync_t** sync);
static void getNotes(note_t** notesArray);
static long skipEvent(void);
static void findMode(char* difficulty, long defaultModePosition);

song_t* parseSongFile(FILE* inputFile, char* difficulty){
  file = inputFile;

  note_t* notes = calloc(notesCurrCap, sizeof(note_t));
  if (!notes) {
    perror("Error in allocating notes");
    exit(EXIT_FAILURE);
  }

  sync_t* syncTrack = calloc(syncCurrCap, sizeof(sync_t));
  if (!syncTrack) {
    perror("Error in allocating sync track");
    exit(EXIT_FAILURE);
  }

  char* name;

  while(1) {
    size_t charLength = 0;
    char* line = NULL;
    ssize_t nread = getline(&line, &charLength, file);

    if (nread < 0) {
      free(line);
      break;
    }

    char* target;
    if ((target = strstr(line, "Name"))) { // Get name
      name = getName(target);
    } else if ((target = strstr(line, "Offset"))) { // Get offset
      sscanf(target, "Offset = %lf\n", &offset);
    } else if ((target = strstr(line, "Resolution"))) { // Get resolution
      sscanf(target, "Resolution = %ld\n", &res);
    } else if ((target = strstr(line, "[SyncTrack]"))) { // Get syncTrack
      getSyncTrack(&syncTrack);
    } else if ((target = strstr(line, "[Events]"))) { 
      // Skip Events Section and find the expected difficulty.
      // If expected difficulty not available, use the first as default
      findMode(difficulty, skipEvent());
      getNotes(&notes);
      free(line);
      break;
    }
    free(line);
  }
  return initSong(name, syncTrack, numOfSync, notes, numOfNotes, res, offset);
}

static void* expandArray(void* array, int* currCap, int elemSize) {
  void* newArray = realloc(array, (*currCap) * 2 * elemSize);
  if (!newArray) {
    perror("Expanding array failed");
    exit(EXIT_FAILURE);
  }
  memset((char*)newArray + (*currCap) * (elemSize), 0, (*currCap) * elemSize);
  (*currCap) *= 2;
  return newArray;
}

static char* getName(char* target){
  char* start = strchr(target, '"');
  char* end = strchr(start + 1, '"');
  char* name = calloc(end - start, sizeof(char));
  if (!name) {
    perror("Failed to allocate memory to store song name");
    exit(EXIT_FAILURE);
  }
  strncpy(name, start + 1, end - start - 1);
  return name;
}

static void getSyncTrack(sync_t** sync) {
  sync_t* syncTrack = *sync;
  fscanf(file, "{\n");
  while (1) {
    long position;
    long bpm;
    int successes = fscanf(file, "\t%ld = B %ld\n", &position, &bpm);
    if (successes == EOF) {
      perror("End of file during sync track");
      exit(EXIT_FAILURE);
    }

    // if successes == 0, then none of the numbers could be matched, 
    // so the line being read must be "}\n", so we break the loop 
    if (successes == 0) {
      fscanf(file, "}\n");
      break;
    }
    // only 1 success, so must be TS line
    if (successes == 1) {
      fscanf(file, "TS %ld\n", &bpm);
      continue;
    }

    //only other case is 2 matches, so must be a bpm line
    addSync(syncTrack + numOfSync, position, bpm);
    numOfSync++;
    // Expand the notes array if there are more notes to read
    if (numOfSync >= syncCurrCap) {
      syncTrack = (sync_t*) expandArray((void*)syncTrack, &syncCurrCap, sizeof(sync_t));
    }
  }
  if (numOfSync <= 0) {
    fprintf(stderr, "No sync track in the chart\n");
    exit(EXIT_FAILURE);
  }
  sync_t* newSyncTrack = realloc(syncTrack, numOfSync * sizeof(sync_t));
  if (!newSyncTrack) {
    perror("Error in resizing sync track");
    exit(EXIT_FAILURE);
  }
  syncTrack = newSyncTrack;
  *sync = syncTrack;
}

static long skipEvent(void) {
  while(1) {
    size_t tempCharLength = 0;
    char* tempLine = NULL;
    getline(&tempLine, &tempCharLength, file);
    if (strchr(tempLine, '}')) {
      free(tempLine);
      break;
    }
    free(tempLine);
  }
  return ftell(file);
}

static void findMode(char* difficulty, long defaultModePosition){
  while(!feof(file)) {
    size_t tempCharLength = 0;
    char* tempLine = NULL;
    getline(&tempLine, &tempCharLength, file);
    if (strstr(tempLine, difficulty)) { // Find the specified mode
      printf("Mode: %s\n", difficulty);
      fscanf(file, "{\n");
      free(tempLine);
      return;
    }
    free(tempLine);
  }
  // Use default mode: the first
  clearerr(file);
  fseek(file, defaultModePosition, SEEK_SET);
  size_t tempCharLength = 0;
  char* tempLine = NULL;
  getline(&tempLine, &tempCharLength, file);
  printf("Default: %s", tempLine);
  free(tempLine); 
  fscanf(file, "{\n");
}

static void getNotes(note_t** notesArray) {
  char* target;
  note_t* notes = *notesArray;
  while(1) {
    size_t charLength = 0;
    char* line = NULL;
    getline(&line, &charLength, file);
    if ((target = strchr(line, '}'))) { // end of loop
      free(line);
      break;
    } else { // get notes
      // Skip notes not with type N      
      if (!strchr(line, 'N')) {
        free(line);
        continue;
      }
      long position;
      int colourNum;
      long length;
      sscanf(line, "\t%ld = N %d %ld\n", &position, &colourNum, &length);
      // Ignore special notes 
      if (colourNum >= MAX_COLOUR) {
        free(line);
        continue;
      }
      initNote(notes + numOfNotes, colourNum, position, length);
      numOfNotes++;
      // Expand the notes array if there are more notes to read
      if (numOfNotes >= notesCurrCap) {
        notes = (note_t*) expandArray((void*)notes, &notesCurrCap, sizeof(note_t));
      }
    }
    free(line);
  }

  // Make the arrays as big as but no bigger than it needs to be
  note_t* newNotes = realloc(notes, numOfNotes * sizeof(note_t));
  if (!newNotes) {
    perror("Resizing notes failed");
    exit(EXIT_FAILURE);
  }
  notes = newNotes;
  *notesArray = notes;
}
