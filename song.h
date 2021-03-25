#ifndef SONG_H
#define SONG_H

#include <stdio.h>

#include "note.h"
#include "sync_track.h"

typedef struct {
  char* name;
  note_t* notes;
  sync_t* syncTrack;
  long resolution;
  double offset;
  int numOfNotes;
  int numOfSync;
} song_t;

song_t* initSong(char* name, sync_t* syncTrack, int numOfSync, note_t* notes,
    int numOfNotes, long res, double offset);
void cleanupSong(song_t* song);
void printSong(song_t* song, FILE* file);

#endif // SONG_H
