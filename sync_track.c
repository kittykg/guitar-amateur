#include <stdio.h>

#include "sync_track.h"

void addSync(sync_t* sync, long positon, long bpm) {
  sync->position = positon;
  sync->bpm = bpm;
}

void printSyncTrack(sync_t* syncTrack, int numOfSync){
  for (int i = 0; i < numOfSync; i++) {
    fprintf(stdout, "%4d: %6ld = B %6ld\n", i,syncTrack[i].position,
        syncTrack[i].bpm);
  }
}
