#include <stdio.h>
#include <stdlib.h>

#include "song.h"

song_t* initSong(char* name, sync_t* syncTrack, int numOfSync, note_t* notes,
    int numOfNotes, long res, double offset){
  song_t* song = malloc(sizeof(song_t));
  if (!song){
    perror("Error in allocating memory for a new song");
    exit(EXIT_FAILURE);
  }
  song->name = name;
  song->notes = notes;
  song->syncTrack = syncTrack;
  song->resolution = res;
  song->offset = offset;
  song->numOfNotes = numOfNotes;
  song->numOfSync = numOfSync;

  return song;
}

void cleanupSong(song_t* song){
  free(song->name);
  free(song->notes);
  free(song->syncTrack);
  free(song);
}

void printSong(song_t* song, FILE* stream){
  fprintf(stream, "Name: %s\nResolution:%ld\nOffset:%f\n",
    song->name, song->resolution, song->offset);
  fprintf(stream, "SyncTrack:\n");
  for (int i = 0; i < song->numOfSync; i++) {
    fprintf(stream, "%4d: %6ld = B %6ld\n", i, song->syncTrack[i].position,
    song->syncTrack[i].bpm);
  }
  fprintf(stream, "Notes:\n");
  for (int i = 0; i < song->numOfNotes; i++) {
    fprintf(stream, "%4d: %6ld = N %d\n", i, song->notes[i].position,
    song->notes[i].colour);
  }
}
