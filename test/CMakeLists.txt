set(CMAKE_C_FLAGS "--coverage ${CMAKE_C_FLAGS}")

add_executable(vinbero_Options_test vinbero_Options_test.c ../src/vinbero_Options.c)
target_link_libraries(vinbero_Options_test vinbero_core vinbero_common cmocka "-Wl,--wrap,vinbero_common_Log_raw -Wl,--wrap,vinbero_common_Log_init -Wl,-wrap,vinbero_Help_print")

add_executable(vinbero_Options_test2 vinbero_Options_test2.c ../src/vinbero_Options.c)
target_link_libraries(vinbero_Options_test2 vinbero_core vinbero_common cmocka "-Wl,--wrap,vinbero_common_Log_raw -Wl,--wrap,vinbero_common_Log_init -Wl,-wrap,vinbero_Help_print")

add_executable(vinbero_core_test vinbero_core_test.c ../src/vinbero_core.c)
target_link_libraries(vinbero_core_test vinbero_core vinbero_common cmocka "-Wl,--wrap,vinbero_common_Log_raw -Wl,--wrap,vinbero_common_Log_init -Wl,-wrap,vinbero_Help_print")

add_test(NAME vinbero_Options_test COMMAND vinbero_Options_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME vinbero_Options_test2 COMMAND vinbero_Options_test2 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME vinbero_core_test COMMAND vinbero_core_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(gcov
  COMMAND gcov vinbero_core_test.c -o "${CMAKE_BINARY_DIR}/test/CMakeFiles/vinbero_core_test.dir"
  COMMAND gcov vinbero_Options_test.c -o "${CMAKE_BINARY_DIR}/test/CMakeFiles/vinbero_Options_test.dir"
  COMMAND gcov vinbero_Options_test2.c -o "${CMAKE_BINARY_DIR}/test/CMakeFiles/vinbero_Options_test2.dir")
