#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "circularBuffer.h"

void circularStringBuffer_init(struct CircularStringBuffer *cb) {
  cb->head = 0;
  cb->tail = 10;
  cb->current_position = 0;
}

//
//Push messages until you get to tail; then print the queue
//then reset the current position to head.
void circularStringBuffer_push(struct CircularStringBuffer *cb, char *message) {

  if (cb->current_position == cb->tail) {
    cb->current_position = cb->head;
  }

  strncpy(cb->buffer[cb->current_position], message, 127);
  cb->buffer[cb->current_position][127] = '\0';
  ++cb->current_position;
}

void circularStringBuffer_print(struct CircularStringBuffer *cb) {
  for (uint8_t i = 0; i < 10; ++i) {
    printf("[%d] %s\n", i, cb->buffer[i]);
  }
}

bool circularStringBuffer_at_tail(struct CircularStringBuffer *cb) {
  return cb->current_position == cb->tail;
}