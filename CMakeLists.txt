cmake_minimum_required(VERSION 3.22)
project(VoxelTutorial)

set(CMAKE_CXX_STANDARD 20)

# Set executable name
set(EXECUTABLE_NAME VoxelTutorial)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Include directories
include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/Resources/Includes
        ${CMAKE_SOURCE_DIR}/Resources/Classes
        ${CMAKE_SOURCE_DIR}/Lib/Glad/include  # GLAD includes
        ${GLFW_INCLUDE_DIRS}
)

# Add GLAD (our OpenGL loader)
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/Lib/Glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/Lib/Glad/include)

# List all source files
set(SOURCES
        main.cpp
        Resources/Classes/Block.cpp
        Resources/Classes/Chunk.cpp
        Resources/Classes/Camera.cpp
        Resources/Classes/WorldGeneration.cpp
        Resources/Classes/World.cpp
        Resources/Classes/Shader.cpp
)

# Create executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})

# Link libraries - NOTE: No GLEW!
target_link_libraries(${EXECUTABLE_NAME}
        OpenGL::GL
        glfw
        glad
)

# Copy shaders to build directory
file(COPY Resources/Shaders DESTINATION ${CMAKE_BINARY_DIR})