#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include <stdio.h>

#include "song.h"

song_t* parseSongFile(FILE* inputFile, char* difficulty);

#endif // FILE_PARSER_H
