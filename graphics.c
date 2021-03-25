#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "graphics.h"
#include "note.h"
#include "game_state.h"
#include "defs.h"

// notes at the current position have a screen y position of SWEETSPOT_Y
#define SWEETSPOT_Y 500  // y pos for perfect timing
#define FRETBOARD_TOP 0
#define FRETBOARD_STARTING_X 275
#define CURRENT_POS (state->currPosition)
#define BEATS_ON_FRETBOARD 4
#define FRETBOARD_HEIGHT 600
#define MS_PER_BEAT (state->currSong->resolution)
#define NOTE_IMG_SIZE 50
#define FONT_SIZE 30
#define SCORE_X 20
#define SCORE_Y 400
#define COMBO_X 550
#define COMBO_Y 400
#define STRING_SIZE 256
#define METER_RANGE 160.0
#define BASE_ROTATION 140.0

static SDL_Texture *loadTexture(const char* filename);
static void drawImage(SDL_Texture* img, int x, int y,
    bool centered, double rotation);
static void drawAllNotes(game_state_t* state);
static void drawBackground();
static void drawInput(game_state_t* state, input_data_t* input);
static void drawFrets(game_state_t* state);
static int getYFromPos(game_state_t* state, int position);
static void drawPaused(game_state_t* state);
static void drawHealth(game_state_t* state);
static void drawScore(game_state_t* state);
static void loadInputTextures();
static void loadNoteTextures();
static void loadTailTextures();
static void drawTail(SDL_Texture* tail, SDL_Texture* tailEnd, int x, int y, int length);
static void drawHits(game_state_t* state);
static void drawCombo(game_state_t* state);
static void drawScoreMultiplier(game_state_t* state);

static SDL_Window* window = NULL;
static SDL_Renderer *renderer = NULL;

static SDL_Texture* fretboard = NULL;
static SDL_Texture* fret = NULL;
static SDL_Texture* sweetspotIndicator = NULL;
static SDL_Texture* pauseTexture = NULL;
static SDL_Texture* dummyNote = NULL;
static SDL_Texture* healthArrow = NULL;
static SDL_Texture* healthFrame = NULL;
static SDL_Texture* hitIndicator = NULL;

static SDL_Texture* inputTextures[MAX_COLOUR] = {NULL};
static SDL_Texture* noteTextures[MAX_COLOUR] = {NULL};

static SDL_Texture* tailTextures[MAX_COLOUR] = {NULL};
static SDL_Texture* tailEndTextures[MAX_COLOUR] = {NULL};
static SDL_Texture* defaultTail = NULL;
static SDL_Texture* defaultTailEnd = NULL;

static TTF_Font* font = NULL;
static SDL_Color fontColour = {255, 255, 255};


void render(game_state_t* state, input_data_t* input) {
  // Clear screen
  SDL_RenderClear(renderer);

  drawBackground();
  drawFrets(state);
  drawImage(sweetspotIndicator, WINDOW_WIDTH / 2, SWEETSPOT_Y, true, 0);
  drawInput(state, input);
  drawAllNotes(state);
  drawHits(state);
  drawHealth(state);
  drawScore(state);
  drawCombo(state);
  drawScoreMultiplier(state);
  drawPaused(state);

  // Update screen
  SDL_RenderPresent(renderer);
}

void initGraphics(void) {
  if (SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    fprintf(stderr, "Unable to initialise SDL video subsystem: %s", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0,
      &window, &renderer)) {
    fprintf(stderr, "Error in creating window and renderer: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  SDL_SetWindowTitle(window, TITLE);

  int flags = IMG_INIT_PNG;
  if (!(IMG_Init(flags) & flags)) {
    printf("Cannot initialise SDL_image: %s\n", IMG_GetError());
    exit(EXIT_FAILURE);
  }
  if (TTF_Init()) {
    printf("Cannot initialise SDL_ttf.");
    exit(EXIT_FAILURE);
  }

  fretboard = loadTexture("./gfx/fretboard.png");
  fret = loadTexture("./gfx/fret.png");
  sweetspotIndicator = loadTexture("./gfx/ssIndicator.png");
  hitIndicator = loadTexture("./gfx/hitFlame.png");
  pauseTexture = loadTexture("./gfx/pause.png");
  dummyNote = loadTexture("./gfx/50x50blackrec.png");
  healthArrow = loadTexture("./gfx/healthArrow.png");
  SDL_SetTextureBlendMode(healthArrow, SDL_BLENDMODE_BLEND);
  healthFrame = loadTexture("./gfx/healthMeter.png");
  loadInputTextures();
  loadNoteTextures();
  loadTailTextures(); // also loads tail ends

  font = TTF_OpenFont("./gfx/"FONT_NAME, FONT_SIZE);
  if(!font) {
    printf("TTF_OpenFont failed: %s\n", TTF_GetError());
    exit(EXIT_FAILURE);
  }
}

void cleanupGraphics() {
  if(fretboard) {
    SDL_DestroyTexture(fretboard);
  }
  if(fret) {
    SDL_DestroyTexture(fret);
  }
  if(sweetspotIndicator) {
    SDL_DestroyTexture(sweetspotIndicator);
  }
  if(hitIndicator) {
    SDL_DestroyTexture(hitIndicator);
  }
  if(dummyNote) {
    SDL_DestroyTexture(dummyNote);
  }
  if(healthArrow) {
    SDL_DestroyTexture(healthArrow);
  }
  if(healthFrame) {
    SDL_DestroyTexture(healthFrame);
  }

  for (int colour = 0; colour < MAX_COLOUR; colour++) {
    if (inputTextures[colour]) {
      SDL_DestroyTexture(inputTextures[colour]);
    }
    if (noteTextures[colour]) {
      SDL_DestroyTexture(noteTextures[colour]);
    }
    if (tailTextures[colour]) {
      SDL_DestroyTexture(tailTextures[colour]);
    }
    if (tailEndTextures[colour]) {
      SDL_DestroyTexture(tailEndTextures[colour]);
    }
  }

  if(defaultTail) {
    SDL_DestroyTexture(defaultTail);
  }
  if(defaultTailEnd) {
    SDL_DestroyTexture(defaultTailEnd);
  }

  TTF_CloseFont(font);
  TTF_Quit();
  IMG_Quit();

  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static SDL_Texture* loadTexture(const char* filename) {
  SDL_Texture* texture = NULL;
  SDL_Surface* image = IMG_Load(filename);
  if (!image) {
    printf("Unable to load image %s: %s\n", filename, IMG_GetError());
    exit(EXIT_FAILURE);
  }

  texture = SDL_CreateTextureFromSurface(renderer, image);
  if (!texture) {
    printf("Unable to create texture from %s : %s\n", filename, SDL_GetError());
    exit(EXIT_FAILURE);
  }

  SDL_FreeSurface(image);
  return texture;
}

static void drawImage(SDL_Texture* img, int x, int y, bool centered, double rotation) {
  // create region rectangle
  SDL_Rect region;
  int width, height;

  // get w, h from image
  if(SDL_QueryTexture(img, NULL, NULL, &width, &height)) {
    printf("QueryTexture failed: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  region.x = x;
  region.y = y;
  if (centered) {
    region.x -= width / 2;
    region.y -= height / 2;
  }
  region.w = width;
  region.h = height;

  // draw image
  if (rotation) {  //used for healthArrow
    SDL_Point pivot;
    pivot.x = 0;
    pivot.y = region.h;
    if (SDL_RenderCopyEx(renderer, img, NULL, &region, rotation, &pivot,
          SDL_FLIP_NONE)) {
      printf("Rendering rotated image failed: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }
  } else {
    if (SDL_RenderCopy(renderer, img, NULL, &region)) {
      printf("Rendering image failed: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }
  }
}

static void drawBackground() {
  // 250 px wide fretboard, starting at x=275, y=0, kept in one 800x600 img
  drawImage(fretboard, 0, FRETBOARD_TOP, false, 0);
}

static void drawFrets(game_state_t* state) {
  int offset = CURRENT_POS % MS_PER_BEAT;
  for(int i = 0; i <= BEATS_ON_FRETBOARD; i++) {
    int y = getYFromPos(state, CURRENT_POS + i * MS_PER_BEAT - offset);
    drawImage(fret, FRETBOARD_STARTING_X, y, false, 0);
  }
}

static void drawInput(game_state_t* state, input_data_t* input) {
  int startingPoint = FRETBOARD_STARTING_X + NOTE_IMG_SIZE / 2;

  for (int colour = 0; colour < MAX_COLOUR; colour++) {
    int x = colour * NOTE_IMG_SIZE + startingPoint;
    if (*(&input->greenDown + colour)) {
      drawImage(noteTextures[colour], x, SWEETSPOT_Y, true, 0);
    }
  }
}

static void drawNote(note_t* note, game_state_t* state) {
  assert(note);
  assert(0 <= note->colour);
  if (note->colour >= MAX_COLOUR) {
    printf("Current  note:%ld N = %d\n", note->position, note->colour);
    printf("Previous note:%ld N = %d\n", (note - 1)->position, (note - 1)->colour);
  }
  assert(note->colour < MAX_COLOUR);

  int x =
    FRETBOARD_STARTING_X + NOTE_IMG_SIZE / 2 + note->colour * NOTE_IMG_SIZE;
  int y = getYFromPos(state, note->position);

  if (note->length > 0) {
    int length = y - getYFromPos(state, note->position + note->length);
    if (note->status == IN_PROGRESS) {
      drawTail(tailTextures[note->colour], tailEndTextures[note->colour], x, y, length);
    } else {
      drawTail(defaultTail, defaultTailEnd, x, y, length);
    }
  }
  drawImage(noteTextures[note->colour], x, y, true, 0);
}

static void drawPaused(game_state_t* state) {
  if(state->isGamePaused) {
    drawImage(pauseTexture, 0, 0, false, 0);
  }
}

static void drawHealth(game_state_t* state) {
  drawImage(healthFrame, 25, 100, false, 0);

  static double lastRotation = METER_RANGE - BASE_ROTATION;

  double goalRotation =
    (double) state->health / MAX_HEALTH * METER_RANGE - BASE_ROTATION;
  lastRotation += (goalRotation - lastRotation) * 0.01;
  drawImage(healthArrow, 125, 200 - 5, false, lastRotation);
}

static int getYFromPos(game_state_t* state, int position) {
  return SWEETSPOT_Y - (SWEETSPOT_Y - FRETBOARD_TOP) * (position - CURRENT_POS)
      / (BEATS_ON_FRETBOARD * MS_PER_BEAT);
}

static void drawScore(game_state_t* state) {
  char scoreString[STRING_SIZE];
  snprintf(scoreString, STRING_SIZE * sizeof(char),
      "Score: %d", state->score);
  SDL_Surface* surface = TTF_RenderText_Blended(font, scoreString, fontColour);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

  // create area where to render the text
  SDL_Rect dstRect;
  int w, h;
  if(SDL_QueryTexture(texture, NULL, NULL, &w, &h)) {
    printf("QueryTexture failed: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  dstRect.w = w;
  dstRect.h = h;
  dstRect.x = SCORE_X;
  dstRect.y = SCORE_Y;

  SDL_RenderCopy(renderer, texture, NULL, &dstRect);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
}

static void drawAllNotes(game_state_t* state) {
  if (!state->firstVisibleNote) {
    return;
  }

  int positionAtTopOfScreen = CURRENT_POS + BEATS_ON_FRETBOARD * MS_PER_BEAT;
  for (note_t* note = state->firstVisibleNote;
      note < state->currSong->notes + state->currSong->numOfNotes &&
      note->position < positionAtTopOfScreen + NOTE_IMG_SIZE * 2; note++) {
    drawNote(note, state);
  }
}

static void loadInputTextures() {
  inputTextures[GREEN] = loadTexture("./gfx/greenInput.png");
  inputTextures[RED] = loadTexture("./gfx/redInput.png");
  inputTextures[YELLOW] = loadTexture("./gfx/yellowInput.png");
  inputTextures[BLUE] = loadTexture("./gfx/blueInput.png");
  inputTextures[ORANGE] = loadTexture("./gfx/orangeInput.png");
}

static void loadNoteTextures() {
  noteTextures[GREEN] = loadTexture("./gfx/greenNote.png");
  noteTextures[RED] = loadTexture("./gfx/redNote.png");
  noteTextures[YELLOW] = loadTexture("./gfx/yellowNote.png");
  noteTextures[BLUE] = loadTexture("./gfx/blueNote.png");
  noteTextures[ORANGE] = loadTexture("./gfx/orangeNote.png");
}

static void loadTailTextures() {
  tailTextures[GREEN] = loadTexture("./gfx/greenTail.png");
  tailTextures[RED] = loadTexture("./gfx/redTail.png");
  tailTextures[YELLOW] = loadTexture("./gfx/yellowTail.png");
  tailTextures[BLUE] = loadTexture("./gfx/blueTail.png");
  tailTextures[ORANGE] = loadTexture("./gfx/orangeTail.png");

  tailEndTextures[GREEN] = loadTexture("./gfx/greenTailRounded.png");
  tailEndTextures[RED] = loadTexture("./gfx/redTailRounded.png");
  tailEndTextures[YELLOW] = loadTexture("./gfx/yellowTailRounded.png");
  tailEndTextures[BLUE] = loadTexture("./gfx/blueTailRounded.png");
  tailEndTextures[ORANGE] = loadTexture("./gfx/orangeTailRounded.png");

  defaultTail = loadTexture("./gfx/blackTail.png");
  defaultTailEnd = loadTexture("./gfx/blackTailRounded.png");
}

static void drawTail(SDL_Texture* tail, SDL_Texture* tailEnd, int x, int y, int length) {
  // create region rectangle
  SDL_Rect region;
  int width, height, endWidth, endHeight;

  // get w, h from images
  if(SDL_QueryTexture(tail, NULL, NULL, &width, &height)) {
    printf("QueryTexture failed: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if(SDL_QueryTexture(tailEnd, NULL, NULL, &endWidth, &endHeight)) {
    printf("QueryTexture failed: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  region.x = x - width / 2;
  region.y = y - length;
  region.w = width;
  region.h = length - endHeight;

  if(SDL_RenderCopy(renderer, tail, NULL, &region)) {
    printf("Rendering image failed: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // draw rounded tail end
  region.y = y - length - endHeight;
  region.w = endWidth;
  region.h = endHeight;

  if (SDL_RenderCopy(renderer, tailEnd, NULL, &region)) {
    printf("Rendering image failed: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
}

static void drawHits(game_state_t* state) {
  // iterate through hitTimers, if its >0, show a hit
  for(int colour = 0; colour < MAX_COLOUR; colour++) {
    if(state->hitTimers[colour] > 0 ||
        (state->heldChord != NULL && state->heldChord->colours[colour])) {
      int x = FRETBOARD_STARTING_X + NOTE_IMG_SIZE / 2 + colour * NOTE_IMG_SIZE;
      int y = SWEETSPOT_Y;
      drawImage(hitIndicator, x, y, true, 0);
    }
  }
}

static void drawCombo(game_state_t* state) {
  char comboString[STRING_SIZE];
  if (state->hitCombo >= MIN_COMBO) {
    snprintf(comboString, STRING_SIZE * sizeof(char),
        "Combo: %d", state->hitCombo);
  } else if (state->comboBrokenTimer > 0){
    snprintf(comboString, STRING_SIZE * sizeof(char), "Combo Broken!");
  } else {
    return;
  }
  SDL_Surface* surface = TTF_RenderText_Blended(font, comboString, fontColour);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

  // create area where to render the text
  SDL_Rect dstRect;
  int w, h;
  if(SDL_QueryTexture(texture, NULL, NULL, &w, &h)) {
    printf("QueryTexture failed: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  dstRect.w = w;
  dstRect.h = h;
  dstRect.x = COMBO_X;
  dstRect.y = COMBO_Y;

  SDL_RenderCopy(renderer, texture, NULL, &dstRect);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
}

static void drawScoreMultiplier(game_state_t* state) {
  char multiplierString[STRING_SIZE];
  snprintf(multiplierString, STRING_SIZE * sizeof(char),
      "Score Multiplier: x%d", state->scoreMultiplier);
  SDL_Surface* surface = TTF_RenderText_Blended(font, multiplierString, fontColour);
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

  // create area where to render the text
  SDL_Rect dstRect;
  int w, h;
  if(SDL_QueryTexture(texture, NULL, NULL, &w, &h)) {
    printf("QueryTexture failed: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  dstRect.w = w;
  dstRect.h = h;
  dstRect.x = SCORE_X;
  dstRect.y = SCORE_Y - 50;

  SDL_RenderCopy(renderer, texture, NULL, &dstRect);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
}
