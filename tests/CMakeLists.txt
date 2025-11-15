include(UnityConfig.cmake)
enable_testing()
add_executable(unit_tests
  ../tests/test_main.c
  ../lib/circularBuffer.c
)
target_include_directories(unit_tests PRIVATE ../lib ../tests)
target_link_libraries(unit_tests PRIVATE garage_lib unity)
add_test(NAME unit_tests COMMAND unit_tests)
