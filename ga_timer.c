#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>

#include "ga_timer.h"

ga_timer_t* initTimer(void) {
  ga_timer_t* timer = malloc(sizeof(ga_timer_t));
  if (!timer) {
    perror("Error in creating timer");
    exit(EXIT_FAILURE);
  }
  timer->lastTimeUpdated = 0;
  timer->timePassed = 0;
  timer->delta = 0;
  return timer;
}

void startTimer(ga_timer_t* timer) {
  timer->lastTimeUpdated = SDL_GetTicks();
  timer->timePassed = 0;
  timer->delta = 0;
}

void incrementTimer(ga_timer_t* timer) {
  Uint32 currTimeMillis = SDL_GetTicks();
  timer->delta = currTimeMillis - timer->lastTimeUpdated;
  timer->timePassed += timer->delta;
  timer->lastTimeUpdated = currTimeMillis;
}

void pauseTimer(ga_timer_t* timer) {
  incrementTimer(timer);
}

void unpauseTimer(ga_timer_t* timer) {
  timer->lastTimeUpdated = SDL_GetTicks();
}

void cleanupTimer(ga_timer_t* timer) {
    free(timer);
}
