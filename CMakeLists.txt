cmake_minimum_required (VERSION 2.6)
cmake_policy (SET CMP0014 OLD)

project ("Autochown")
include_directories ("${PROJECT_SOURCE_DIR}/src")
set(CMAKE_C_FLAGS "-Wall")
# set(CMAKE_C_FLAGS "-Wall -g")

add_subdirectory (src)
set (
  RUNTIME_OUTPUT_DIRECTORY
  "${PROJECT_BINARY_DIR}/bin"
)


add_executable (
  autochown
  src/main.c
  src/common.c
  src/file_parser.c
  src/inotify.c
)

install (
  PROGRAMS "${PROJECT_BINARY_DIR}/autochown"
  DESTINATION bin
)
