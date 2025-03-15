# cmake -S . -B build -G "MinGW Makefiles" cmake --build build
cmake_minimum_required(VERSION 3.10) # Updated minimum version
project(MyGameFramework)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# GLFW
set(GLFW3_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/glfw-3.4")
set(GLFW3_INCLUDE_DIR "${GLFW3_ROOT}/include")
set(GLFW3_LIBRARY "${GLFW3_ROOT}/build/src/libglfw3.a")
include_directories(${GLFW3_INCLUDE_DIR})

# GLEW
add_definitions(-DGLEW_STATIC)

set(GLEW_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/glew-2.1.0")
set(GLEW_INCLUDE_DIR "${GLEW_ROOT}/include")
set(GLEW_LIBRARY "${GLEW_ROOT}/build/cmake/build/lib/libglew32.a")
include_directories(${GLEW_INCLUDE_DIR})

# FreeImage
set(FREEIMAGE_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/FreeImage")
set(FREEIMAGE_INCLUDE_DIR "${FREEIMAGE_ROOT}/Dist")
set(FREEIMAGE_LIBRARY "${FREEIMAGE_ROOT}/Dist/libfreeimage.a")
include_directories(${FREEIMAGE_INCLUDE_DIR})
if(NOT EXISTS "${FREEIMAGE_LIBRARY}")
    message(FATAL_ERROR "FreeImage static library not found at ${FREEIMAGE_LIBRARY}")
endif()
add_definitions(-DFREEIMAGE_LIB) # Define for static linking

# miniaudio
set(MINIAUDIO_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/miniaudio")
set(MINIAUDIO_INCLUDE_DIR "${MINIAUDIO_ROOT}")
include_directories(${MINIAUDIO_INCLUDE_DIR})

# Lua
set(LUA_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/build/lua-5.4.7")
set(LUA_INCLUDE_DIR "C:/Users/barte/Desktop/VoltaFramework/include/lua-5.4.7/src")
set(LUA_LIBRARY "${LUA_ROOT}/liblua_static.a")
include_directories(${LUA_INCLUDE_DIR})

# Source files
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(SOURCES
    ${SOURCE_DIR}/VoltaFramework.cpp
    ${SOURCE_DIR}/Window.cpp
    ${SOURCE_DIR}/Audio.cpp
    ${SOURCE_DIR}/Graphics.cpp
    ${SOURCE_DIR}/Input.cpp
    ${SOURCE_DIR}/MathUtils.cpp
    ${SOURCE_DIR}/TableUtils.cpp
    ${SOURCE_DIR}/Buffer.cpp
    ${SOURCE_DIR}/Vector2.cpp
    ${SOURCE_DIR}/Event.cpp
    ${SOURCE_DIR}/ParticleEmitter.cpp
    ${SOURCE_DIR}/Json.cpp
    ${SOURCE_DIR}/Filesystem.cpp
    ${SOURCE_DIR}/main.cpp
)

include_directories(${OPENGL_INCLUDE_DIRS} include)

foreach(file ${SOURCES})
    if(NOT EXISTS "${file}")
        message(FATAL_ERROR "Source file '${file}' not found")
    endif()
endforeach()

add_executable(game ${SOURCES})
target_link_libraries(game
    ${GLFW3_LIBRARY}
    ${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}  
    ${LUA_LIBRARY}
    ${FREEIMAGE_LIBRARY}
    opengl32
    ws2_32
)
target_compile_features(game PUBLIC cxx_std_17)


file(COPY scripts assets DESTINATION ${CMAKE_BINARY_DIR})