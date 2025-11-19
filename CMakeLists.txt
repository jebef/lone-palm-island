cmake_minimum_required(VERSION 3.26)
project(LonePalmIsland LANGUAGES CXX)

# -----------------------------
# Set C++ standard
# -----------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# Add the source files
# -----------------------------
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/utils/camera.cpp
    src/utils/shader.cpp
    src/utils/stb_image.cpp
    src/utils/stb_image_write.cpp
    src/utils/core.cpp
    src/utils/water_frame_buffers.cpp
)

# -----------------------------
# Enable folder grouping in IDEs
# -----------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# -----------------------------
# vcpkg dependencies
# find_package tells CMake:
# "Ask vcpkg to find this library"
# -----------------------------
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

# -----------------------------
# Link libraries to your executable
# -----------------------------
target_link_libraries(${PROJECT_NAME}
    PRIVATE glad::glad
    PRIVATE glfw
    PRIVATE glm::glm
    PRIVATE assimp::assimp
)

# -----------------------------
# Recommended compile warnings
# -----------------------------
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4) # add back /WX to treat all warnings as errors 
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -----------------------------
# Copy shaders to build folder
# -----------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

# -----------------------------
# Copy assets to build folder
# -----------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
