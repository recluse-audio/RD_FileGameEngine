cmake_minimum_required(VERSION 3.15)
project(RD_FILE_GAME_ENGINE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CMAKE/SOURCES.cmake)
include(CMAKE/TESTS.cmake)

include(FetchContent)

# --- Tests target ---------------------------------------------------
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.1.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(Tests
    ${ENGINE_SOURCES}
    ${TEST_SOURCES}
)

target_include_directories(Tests PRIVATE
    SOURCE/SHARED
    THIRD_PARTY
)

target_link_libraries(Tests PRIVATE
    Catch2::Catch2WithMain
)

# --- Raylib desktop target (opt-in) ---------------------------------
option(BUILD_RAYLIB "Build the Raylib desktop target" OFF)

if(BUILD_RAYLIB)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
    )
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(raylib)

    add_executable(GameEngine_Raylib
        SOURCE/RAYLIB/main.cpp
        SOURCE/RAYLIB/RaylibGraphicsRenderer.cpp
        SOURCE/RAYLIB/GameFolderBrowser.cpp
        SOURCE/SHARED/SCENE/nanosvg_impl.cpp
        ${ENGINE_SOURCES}
    )

    target_include_directories(GameEngine_Raylib PRIVATE
        SOURCE/SHARED
        SOURCE/RAYLIB
        THIRD_PARTY
    )

    target_link_libraries(GameEngine_Raylib PRIVATE
        raylib
    )
endif()
