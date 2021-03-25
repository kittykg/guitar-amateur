#ifndef DEFS_H
#define DEFS_H

#define TITLE "Guitar Amateur"

#define SFX_FOLDER "./sfx/"

#define MUSIC_FILENAME "/song.ogg"
#define CHART_FILENAME "/notes.chart"
#define MISS_SAMPLE_FILENAME "/oof.ogg"

#define FONT_NAME "/IHateComicSans.ttf"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#ifdef EASY_MODE
#define PENALTY 0
#else
#define PENALTY 4
#endif

#define REWARD_HEALTH 2
#define REWARD_SCORE (state->scoreMultiplier * 1000)
#define MIN_COMBO 10

#endif // DEFS_H
