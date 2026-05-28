#!/usr/bin/env bash

set -e

mkdir -p build
pushd build > /dev/null

SDLInclude="-I../libraries/SDL-release-3.4.8/include"
GLADInclude="-I../libraries/glad/include"
GLMInclude="-I../libraries/glm-1.0.1-light"
STBInclude="-I../libraries/stb"
# JSONInclude="-I../libraries/json"

IncludeDirectories="$SDLInclude $GLADInclude $GLMInclude $STBInclude $JSONInclude"

echo
echo "COPYING ASSETS TO BUILD DIRECTORY..."
cp -ru ../assets ./assets
echo

echo "COPYING SHADERS TO BUILD DIRECTORY..."
cp -ru ../shaders ./shaders
echo

echo "BUILDING..."
echo

# Debug build
clang++ -g -O0 -Wall -Wextra -Werror \
    ../src/scoundrel.cpp \
    ../src/log.cpp \
    ../src/application.cpp \
    ../src/renderer.cpp \
    ../src/camera.cpp \
    ../src/texture.cpp \
    ../src/mouse.cpp \
    ../src/keyboard.cpp \
    ../src/audio.cpp \
    -x c ../libraries/glad/src/glad.c \
    $IncludeDirectories \
    #-L../libraries/SDL3-3.2.10/build \
    -lSDL3 \
    -o scoundrel

popd > /dev/null
