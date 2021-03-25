#include <assert.h>

#include "note.h"

void initNote(note_t* note, int colNum, long position, long length){
  assert(0 <= colNum && colNum < MAX_COLOUR);
  note->status = AVAILABLE;
  note->colour = colNum;
  note->position = position;
  note->length = length;
}
