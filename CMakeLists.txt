# cmake -S . -B build -G "MinGW Makefiles" && cmake --build build
cmake_minimum_required(VERSION 3.10)
project(VoltaFramework)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

set(GLFW3_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/glfw-3.4")
set(GLFW3_INCLUDE_DIR "${GLFW3_ROOT}/include")
set(GLFW3_LIBRARY "${GLFW3_ROOT}/build/src/libglfw3.a")
include_directories(${GLFW3_INCLUDE_DIR})

add_definitions(-DGLEW_STATIC)
set(GLEW_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/glew-2.1.0")
set(GLEW_INCLUDE_DIR "${GLEW_ROOT}/include")
set(GLEW_LIBRARY "${GLEW_ROOT}/lib/libglew32.a")
include_directories(${GLEW_INCLUDE_DIR})

set(FREEIMAGE_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/FreeImage")
set(FREEIMAGE_INCLUDE_DIR "${FREEIMAGE_ROOT}/Dist/x32")
set(FREEIMAGE_LIBRARY "${FREEIMAGE_ROOT}/Dist/x32/FreeImage.lib")
set(FREEIMAGE_DLL "${FREEIMAGE_ROOT}/Dist/x32/FreeImage.dll")
include_directories(${FREEIMAGE_INCLUDE_DIR})

set(MINIAUDIO_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/miniaudio")
set(MINIAUDIO_INCLUDE_DIR "${MINIAUDIO_ROOT}")
include_directories(${MINIAUDIO_INCLUDE_DIR})

set(LUA_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/build/lua-5.4.7")
set(LUA_INCLUDE_DIR "C:/Users/barte/Desktop/VoltaFramework/include/lua-5.4.7/src")
set(LUA_LIBRARY "${LUA_ROOT}/liblua_static.a")
include_directories(${LUA_INCLUDE_DIR})

set(SQLITE3_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/sqlite")
set(SQLITE3_INCLUDE_DIR "${SQLITE3_ROOT}")
set(SQLITE3_LIBRARY "${SQLITE3_ROOT}/sqlite3.lib")
set(SQLITE3_DLL "${SQLITE3_ROOT}/sqlite3.dll")
include_directories(${SQLITE3_INCLUDE_DIR})

if(NOT EXISTS "${SQLITE3_LIBRARY}")
    message(FATAL_ERROR "SQLite3 library not found at ${SQLITE3_LIBRARY}")
endif()
if(NOT EXISTS "${SQLITE3_DLL}")
    message(WARNING "SQLite3 DLL not found at ${SQLITE3_DLL}. Ensure it’s available at runtime.")
endif()

set(FREETYPE_ROOT "C:/Users/barte/Desktop/VoltaFramework/include/freetype-2.13.3")
set(FREETYPE_INCLUDE_DIR "${FREETYPE_ROOT}/include")
set(FREETYPE_LIBRARY "${FREETYPE_ROOT}/build/libfreetype.a")
include_directories(${FREETYPE_INCLUDE_DIR})

if(NOT EXISTS "${FREETYPE_LIBRARY}")
    message(FATAL_ERROR "FreeType static library not found at ${FREETYPE_LIBRARY}")
endif()

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/scripts")

set(FRAMEWORK_SOURCES
    ${SOURCE_DIR}/VoltaFramework.cpp
    ${SOURCE_DIR}/LuaAPI.cpp
    ${SOURCE_DIR}/Window.cpp
    ${SOURCE_DIR}/Audio.cpp
    ${SOURCE_DIR}/Graphics.cpp
    ${SOURCE_DIR}/Input.cpp
    ${SOURCE_DIR}/MathUtils.cpp
    ${SOURCE_DIR}/TableUtils.cpp
    ${SOURCE_DIR}/Camera2D.cpp
    ${SOURCE_DIR}/Buffer.cpp
    ${SOURCE_DIR}/Vector2.cpp
    ${SOURCE_DIR}/Vector3.cpp
    ${SOURCE_DIR}/Matrix.cpp
    ${SOURCE_DIR}/Color.cpp
    ${SOURCE_DIR}/Event.cpp
    ${SOURCE_DIR}/ParticleEmitter.cpp
    ${SOURCE_DIR}/Json.cpp
    ${SOURCE_DIR}/Sqlite.cpp
    ${SOURCE_DIR}/Filesystem.cpp
)

include_directories(${OPENGL_INCLUDE_DIRS} include ${SOURCE_DIR} ${SCRIPTS_DIR})

foreach(file ${FRAMEWORK_SOURCES})
    if(NOT EXISTS "${file}")
        message(FATAL_ERROR "Framework source file '${file}' not found")
    endif()
endforeach()

add_library(VoltaFramework STATIC ${FRAMEWORK_SOURCES})
target_link_libraries(VoltaFramework
    ${GLFW3_LIBRARY}
    ${GLEW_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${LUA_LIBRARY}
    ${FREEIMAGE_LIBRARY}
    ${SQLITE3_LIBRARY}
    ${FREETYPE_LIBRARY}
    opengl32
    ws2_32
)
target_compile_features(VoltaFramework PUBLIC cxx_std_17)

set(MAIN_SRC "${SOURCE_DIR}/main.cpp")
set(SCRIPTS_MAIN_SRC "${SCRIPTS_DIR}/main.cpp")

if(NOT EXISTS "${MAIN_SRC}")
    message(FATAL_ERROR "src/main.cpp not found at ${MAIN_SRC}")
endif()

set(GAME_SOURCES ${MAIN_SRC})

if(EXISTS "${SCRIPTS_MAIN_SRC}")
    message(STATUS "Found scripts/main.cpp - including C++ game logic")
    set(GAME_SOURCES ${GAME_SOURCES} ${SCRIPTS_MAIN_SRC})
    add_definitions(-DSCRIPTS_MAIN_EXISTS)
    if(NOT EXISTS "${SCRIPTS_DIR}/Game.hpp")
        file(WRITE "${SCRIPTS_DIR}/Game.hpp"
            "#ifndef GAME_HPP\n"
            "#define GAME_HPP\n"
            "#include \"VoltaFramework.hpp\"\n"
            "class Game : public GameBase {\n"
            "public:\n"
            "    void init(VoltaFramework* framework) override;\n"
            "    void update(VoltaFramework* framework, float dt) override;\n"
            "};\n"
            "#endif\n"
        )
        message(STATUS "Generated scripts/Game.hpp")
    endif()
else()
    message(STATUS "No scripts/main.cpp found - Lua-only mode enabled")
endif()

add_executable(game ${GAME_SOURCES})
target_link_libraries(game VoltaFramework)
target_compile_features(game PUBLIC cxx_std_17)

add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FREEIMAGE_DLL}"
    $<TARGET_FILE_DIR:game>
)
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SQLITE3_DLL}"
    $<TARGET_FILE_DIR:game>
)

file(COPY scripts assets DESTINATION ${CMAKE_BINARY_DIR})