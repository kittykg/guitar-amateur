#ifndef GA_TIMER_H
#define GA_TIMER_H

#include <SDL2/SDL.h>

typedef struct {
  Uint32 timePassed;
  Uint32 lastTimeUpdated;
  Uint32 delta;
} ga_timer_t;

ga_timer_t* initTimer(void);
void cleanupTimer(ga_timer_t* timer);
void startTimer(ga_timer_t* timer);
void incrementTimer(ga_timer_t* timer);
void pauseTimer(ga_timer_t* timer);
void unpauseTimer(ga_timer_t* timer);

#endif // GA_TIMER_H
