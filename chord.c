#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "chord.h"

void initChord(note_t* firstNote, chord_t* chord) {
  chord->startNote = firstNote;
  chord->numOfNotes = 0;
  memset(chord->colours, 0, sizeof(bool) * MAX_COLOUR);
  addNoteToChord(firstNote, chord);
}

void addNoteToChord(note_t* note, chord_t* chord) {
  if (chord->colours[note->colour]) {
    printChord(chord);
    fprintf(stderr, "Same note added to chord twice\n");
    exit(EXIT_FAILURE);
  }
  chord->colours[note->colour] = true;
  chord->numOfNotes += 1;
}

void printChord(chord_t* chord) {
  printf("Position:%ld\n", chord->startNote->position);
  printf("NumOfNotes:%d\n", chord->numOfNotes);
  printf("Notes: [");
  for (int i = 0; i < MAX_COLOUR; i++) {
    printf("%d ",chord->colours[i]);
  }
  printf("\b]\n");
}

chord_t* cloneChord(chord_t* target) {
  chord_t* copy = malloc(sizeof(chord_t));
  if (!copy) {
    perror("Clone chord failed");
    exit(EXIT_FAILURE);
  }
  memcpy(copy, target, sizeof(chord_t));
  return copy;
}
