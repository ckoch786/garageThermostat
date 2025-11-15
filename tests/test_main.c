#include "unity.h"
#include "circularBuffer.h"

void setUp(void) {}
void tearDown(void) {}

void test_CircularBuffer_Init(void) {
    struct CircularStringBuffer buf;
    circularStringBuffer_init(&buf);
    TEST_ASSERT_EQUAL(buf.head, 0);
    TEST_ASSERT_EQUAL(buf.tail, 0);
    TEST_ASSERT_EQUAL(buf.current_position, 0);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_CircularBuffer_Init);
    return UNITY_END();
}
