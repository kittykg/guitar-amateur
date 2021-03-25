#ifndef NOTE_H
#define NOTE_H

typedef enum {HIT, MISS, AVAILABLE, IN_PROGRESS} note_status_t;

typedef enum {GREEN, RED, YELLOW, BLUE, ORANGE, MAX_COLOUR} note_colour_t;

typedef struct {
  note_status_t status;
  note_colour_t colour;
  long position;
  long length;
} note_t;

void initNote(note_t* note, int colNum, long position, long length);

#endif // NOTE_H
