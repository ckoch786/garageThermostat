// // Circular Buffer or Ring Buffer?
// // [0, 1 , 2, ... , 10]
// //  ^  ^             ^
// //  |  |             |
// //  |  |             |
// //  |  |             |--- tail
// //  |  |--- current position
// //  |--- head
// //

// #include <stdio.h>
// #include <stdint.h>
// #include <string.h>


// struct CircularStringBuffer {
// 	uint8_t head;
// 	uint8_t tail;
// 	uint8_t current_position;
// 	// define 10 128 character strings.
// 	char buffer[10][128];
// };

// void circularStringBuffer_init(struct CircularStringBuffer *cb) {
// 	cb->head = 0;
// 	cb->tail = 10;
// 	cb->current_position = 0;
// }



// //
// //Push messages until you get to tail; then print the queue
// //then reset the current position to head.
// void circularStringBuffer_push(struct CircularStringBuffer *cb, char *message) {

// 	if (cb->current_position == cb->tail) {
// 		cb->current_position = cb->head;
// 	}

// 	strncpy(cb->buffer[cb->current_position], message, 127);
// 	cb->buffer[cb->current_position][127] = '\0';
// 	++cb->current_position;
// }

// void circularStringBuffer_print(struct CircularStringBuffer *cb) {
// 	for (uint8_t i = 0; i < 10; ++i) {
// 		printf("[%d] %s\n", i, cb->buffer[i]);
// 	}
// }


// int main(void) {
// 	printf("Initializing...\n");
// 	struct CircularStringBuffer buffer;
// 	circularStringBuffer_init(&buffer);
// 	circularStringBuffer_push(&buffer, "Test message one.");  // eleven
// 	printf("%s\n", buffer.buffer[0]);
// 	circularStringBuffer_push(&buffer, "Test message two.");  // twelve
// 	circularStringBuffer_push(&buffer, "Test message three.");
// 	circularStringBuffer_push(&buffer, "Test message four.");
// 	circularStringBuffer_push(&buffer, "Test message five.");
// 	circularStringBuffer_push(&buffer, "Test message six.");
// 	circularStringBuffer_push(&buffer, "Test message seven.");
// 	circularStringBuffer_push(&buffer, "Test message eight.");
// 	circularStringBuffer_push(&buffer, "Test message nine.");
// 	circularStringBuffer_push(&buffer, "Test message ten.");     // overflow
// 	circularStringBuffer_push(&buffer, "Test message eleven.");  // 0
// 	circularStringBuffer_push(&buffer, "Test message twelve.");  //1
// 	circularStringBuffer_print(&buffer);
// }
