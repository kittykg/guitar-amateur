#ifndef CHORD_H
#define CHORD_H

#include <stdbool.h>

#include "note.h"

typedef struct {
  note_t* startNote;
  int numOfNotes;
  bool colours[MAX_COLOUR];
} chord_t;

void initChord(note_t* firstNote, chord_t* chord);
void addNoteToChord(note_t* note, chord_t* chord);
void printChord(chord_t* chord);
chord_t* cloneChord(chord_t* target);

#endif // CHORD_H
