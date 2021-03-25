#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

void initAudio(char* songFolder);
void cleanupAudio(void);

void playMusic(void);
void pauseMusic(void);
void resumeMusic(void);
bool musicIsOver(void);

void playMissSound(void);

#endif // AUDIO_H
