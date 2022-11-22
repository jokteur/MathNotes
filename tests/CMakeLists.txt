add_executable(${PROJECT_NAME}_tests ${sources} main.cpp)
target_link_libraries(${PROJECT_NAME}_tests doctest::doctest ${PROJECT_NAME}_lib)

add_custom_command(TARGET ${PROJECT_NAME}_tests PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:${PROJECT_NAME}_tests>) 