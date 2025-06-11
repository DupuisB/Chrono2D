# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name and Language
project(SFMLBlobGame LANGUAGES CXX)

# Set C++ Standard (C++17 is used)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF) # Enforce standard C++ without GNU extensions

# --- Configure SFML Build (Optional: customize which SFML modules are built if building from source) ---
# These options are useful if SFML is being built as part of this project (e.g., via add_subdirectory).
# If SFML is pre-installed and found via find_package, these might not have an effect.
# set(SFML_BUILD_AUDIO OFF CACHE BOOL "Disable building sfml-audio")
# set(SFML_BUILD_NETWORK OFF CACHE BOOL "Disable building sfml-network")
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "Disable building SFML examples") # Recommended to speed up SFML build
set(SFML_BUILD_TESTING OFF CACHE BOOL "Disable building SFML tests")    # Recommended to speed up SFML build
set(SFML_USE_JOYSTICK_UDEV OFF CACHE BOOL "Disable using UDev for Joystick support (Linux specific)")

# --- Add SFML Source Directory ---
# Builds SFML from the 'dep/sfml' subdirectory.
# EXCLUDE_FROM_ALL prevents SFML targets from being built by default with a global "make" or "build all",
# but they will be built if they are dependencies of your main executable.
add_subdirectory(dep/sfml EXCLUDE_FROM_ALL)

# ---- Add Box2d Source Directory ---
# Builds Box2D from the 'dep/box2d' subdirectory and defines the 'box2d' target.
add_subdirectory(dep/box2d)


# --- Define Your Game Executable ---
# Creates an executable named 'sfml_blob' from main.cpp and src/game_object.cpp.
add_executable(sfml_blob main.cpp src/game_object.cpp src/player.cpp)

# --- Add Include Directory ---
# Specifies the directory where header files (e.g., constants.hpp, utils.hpp, game_object.hpp) are located.
target_include_directories(sfml_blob PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# --- Link Your Executable Against SFML and Box2D Libraries ---
# Links the 'sfml_blob' executable with the necessary SFML modules and the Box2D library.
target_link_libraries(sfml_blob PRIVATE sfml-graphics sfml-window sfml-system sfml-audio box2d)