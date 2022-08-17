# Set C++ 17 compiler flags
cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)

# Set project name
project(MathNotes)

# ---- Tempo app ----
add_subdirectory(external/TempoApp)
include_directories(external/TempoApp/include)
include_directories(external/TempoApp/external/imgui/imgui)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Tempo)

# Copy the data (such as fonts) in build directory
add_custom_command(TARGET MathNotes PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:MathNotes>)

# Set compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()