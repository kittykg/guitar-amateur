#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "audio.h"
#include "defs.h"

#define NUM_AUDIO_CHANNELS 1
#define AUDIO_CHUNKSIZE 1024

#define PLAY_ONCE 1

enum {
  MISS_SAMPLE_CHANNEL,
  NUM_CHANNELS
};

static void setMusicOver(void);
static void loadMusic(char* songFolder);
static void loadMissSample();

static Mix_Music* music = NULL;
static Mix_Chunk* missSample = NULL;
static bool musicOver = false;

void initAudio(char* songFolder) {
  if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
    fprintf(stderr, "Failed to init audio subsystem: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
        NUM_AUDIO_CHANNELS, AUDIO_CHUNKSIZE)) {
    fprintf(stderr, "Failed to open an audio device: %s\n", Mix_GetError());
    exit(EXIT_FAILURE);
  }

  int flags = MIX_INIT_OGG;
  if ((Mix_Init(flags) & flags) != flags) {
    fprintf(stderr, "Failed to init ogg support: %s\n", Mix_GetError());
    exit(EXIT_FAILURE);
  }

  loadMusic(songFolder);

  Mix_HookMusicFinished(setMusicOver);

  loadMissSample(songFolder);

  Mix_AllocateChannels(NUM_CHANNELS);
}

void playMusic() {
  if (Mix_PlayMusic(music, PLAY_ONCE)) {
    fprintf(stderr, "Mix_PlayMusic: %s\n", Mix_GetError());
    exit(EXIT_FAILURE);
  }
}

void pauseMusic() {
  Mix_PauseMusic();
}

bool musicIsOver(void) {
  return musicOver;
}

void setMusicOver(void) {
  musicOver = true;
}

void resumeMusic() {
  Mix_ResumeMusic();
}

void playMissSound() {
  Mix_HaltChannel(MISS_SAMPLE_CHANNEL);
  // Loop the sample 0 times (i.e. play once)
  Mix_PlayChannel(MISS_SAMPLE_CHANNEL, missSample, 0);
}

void cleanupAudio(void) {
  if (music) {
    Mix_FreeMusic(music);
    music = NULL;
  }

  if (missSample) {
    Mix_FreeChunk(missSample);
    missSample = NULL;
  }

  Mix_Quit();
  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static void loadMusic(char* songFolder) {
  char musicFilename[] = MUSIC_FILENAME;
  int size = strlen(songFolder) + strlen(musicFilename) + 1;
  char musicPath[size];

  strcpy(musicPath, songFolder);
  strcat(musicPath, musicFilename);

  music = Mix_LoadMUS(musicPath);
  if (!music) {
    fprintf(stderr, "Failed to open music file: %s\n", Mix_GetError());
    exit(EXIT_FAILURE);
  }
}

static void loadMissSample() {
  char* samplePath = SFX_FOLDER MISS_SAMPLE_FILENAME;

  Mix_Volume(MISS_SAMPLE_CHANNEL, MIX_MAX_VOLUME/3);
  missSample = Mix_LoadWAV(samplePath);
  if (!missSample) {
    fprintf(stderr, "Failed to open sample file: %s \n", Mix_GetError());
    exit(EXIT_FAILURE);
  }
}

