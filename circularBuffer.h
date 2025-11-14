#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

struct CircularStringBuffer {
  uint8_t head;
  uint8_t tail;
  uint8_t current_position;
  // define 10 128 character strings.
  char buffer[10][128];
};

void circularStringBuffer_init(struct CircularStringBuffer *cb);
void circularStringBuffer_push(struct CircularStringBuffer *cb, char *message);
void circularStringBuffer_print(struct CircularStringBuffer *cb);
bool circularStringBuffer_at_tail(struct CircularStringBuffer *cb);
#endif