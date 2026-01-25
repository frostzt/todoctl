cmake_minimum_required(VERSION 3.16)
project(todoctl LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
  -Wall -Wextra -Werror -Wpedantic
  -O0 -g
)

# Add sanitizer for debug build
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
  add_compile_definitions(DEBUG)
endif()

# ---------- Core Library ----------
add_library(todoctl_core STATIC
  src/db.c
  src/entry.c
  src/util.c
)

target_include_directories(todoctl_core PUBLIC include)

# ---------- Main Exec ----------
add_executable(todoctl src/main.c)
target_link_libraries(todoctl PRIVATE todoctl_core)

