cmake_minimum_required(VERSION 2.8)
project(vinbero C)

set(CMAKE_C_OUTPUT_EXTENSION_REPLACE ON)
set(vinbero_version "0.2.0")
set(vinbero_soversion "0.2")

if(CMAKE_VERSION VERSION_LESS "3.1")
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "-std=gnu11 ${CMAKE_C_FLAGS}")
  endif()
else()
    set(CMAKE_C_STANDARD 11)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose CMAKE_BUILD_TYPE" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "MinSizeRel" "Release" "RelWithDebInfo")
endif()

set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")
configure_file("${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}_Version.h.in"
               "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_Version.h" @ONLY)
include_directories("${PROJECT_BINARY_DIR}")

execute_process(COMMAND sh "-c" "git branch -a | grep remotes/origin/master | wc -l" ERROR_QUIET OUTPUT_VARIABLE master_branch_exists)
if(master_branch_exists EQUAL 1)
  message("Branch origin/master exists, enabling automatic AUTHORS generation")
  execute_process(COMMAND git shortlog -nse --no-merges origin/master OUTPUT_FILE "${CMAKE_SOURCE_DIR}/AUTHORS")
  add_custom_target(authors ALL git shortlog -nse --no-merges origin/master > "${CMAKE_SOURCE_DIR}/AUTHORS")
endif()

add_subdirectory(src)
enable_testing()
add_subdirectory(test)

