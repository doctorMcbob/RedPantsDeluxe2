#include "floatdata.h"

/**
   Floats
There will be room for 16 floats per statement.

In evaluating a statement, the float buffer will be cleared.
Literal floats will be pulled from floatdata.h and put into the buffer
floats referenced from
*/

float FLOAT_BUFFER[16] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

int FLOATS_IN_BUFFER = 0;

void clear_float_buffer() {
  for (int i = 0; i < 16; i++) {
    FLOAT_BUFFER[i] = 0.0;
  }
  FLOATS_IN_BUFFER = 0;
}

int push_float(float f) {
  FLOAT_BUFFER[FLOATS_IN_BUFFER] = f;
  return FLOATS_IN_BUFFER++;
}

float get_float_literal(int idx) { return FLOATS[idx]; }

float get_float(int idx) { return FLOAT_BUFFER[idx]; }
