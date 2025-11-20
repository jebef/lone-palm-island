# cmake version + project definition 
cmake_minimum_required(VERSION 3.26)
project(LonePalmIsland LANGUAGES CXX)

# specify c++ standards 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add source files 
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/utils/camera.cpp
    src/utils/shader.cpp
    src/utils/stb_image.cpp
    src/utils/stb_image_write.cpp
    src/utils/core.cpp
    src/utils/water_frame_buffers.cpp
)

# enable folder grouping 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# tell vcpkg which dependencies to find 
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

# link dependencies to executable
target_link_libraries(${PROJECT_NAME}
    PRIVATE glad::glad
    PRIVATE glfw
    PRIVATE glm::glm
    PRIVATE assimp::assimp
)

# compile warning preferences 
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# copy shaders to build folder 
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

# copy assets to build folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
