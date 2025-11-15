add_library(garage_lib
  circularBuffer.c
  # Add other portable, testable C files here
)
target_include_directories(garage_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
