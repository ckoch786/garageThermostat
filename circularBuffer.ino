// Circular Buffer or Ring Buffer?
// [0, 1 , 2, 3]
//  ^  ^      ^
//  |  |      |
//  |  |      |
//  |  |      |--- tail
//  |  |--- current position
//  |--- head
//
// push
// peek

struct CircularStringBuffer {
  uint8_t head = 0;
  uint8_t tail = 10;
  uint8_t current_position = 0;
  // define 10 128 character strings.
  char buffer[10][128];
};


//
// Push messages until you get to tail; then print the queue
// then reset the current position to head.
void circularBuffer_push(struct CircularStringBuffer *cb, char *message) {
  if (cb->current_position == cb->tail) {
    cb->current_position = cb->head;
  }

  cb->current_position = message;
}