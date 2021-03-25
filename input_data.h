#ifndef INPUT_DATA_H
#define INPUT_DATA_H

#include <stdbool.h>

typedef struct {
  bool pausePressed;
  bool strummed;
  bool exitPressed;

  bool greenDown;
  bool redDown;
  bool yellowDown;
  bool blueDown;
  bool orangeDown;
} input_data_t;

input_data_t* initInputData(void);
void clearAllFlags(input_data_t* inputData);
void cleanupInputData(input_data_t* inputData);

#endif // INPUT_DATA_H
