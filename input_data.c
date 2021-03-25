#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input_data.h"

input_data_t* initInputData(void) {
  input_data_t* inputData = malloc(sizeof(input_data_t));
  if (!inputData) {
    perror("Failed to allocate memory for input data");
    exit(EXIT_FAILURE);
  }
  clearAllFlags(inputData);
  return inputData;
}

void clearAllFlags(input_data_t* inputData) {
  memset(inputData, 0, sizeof(input_data_t));
}

void cleanupInputData(input_data_t* inputData) {
  free(inputData);
}
