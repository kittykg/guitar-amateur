#ifndef SYNC_TRACK
#define SYNC_TRACK

typedef struct {
  long position;
  long bpm;
} sync_t;

void addSync(sync_t* sync, long position, long bpm);
void printSyncTrack(sync_t* syncTrack, int numOfSync);

#endif // SYNC_TRACK
