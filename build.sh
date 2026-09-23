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

echo "COPYING ASSETS TO BUILD DIRECTORY..."
cp -ru ../assets/. ./assets/

echo "COPYING SHADERS TO BUILD DIRECTORY..."
cp -ru ../src/shaders/. ./shaders/

echo "BUILDING..."

# Debug build
# -fsanitize=address,undefined,integer
clang++ -g -O0 -Wall -Wextra -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-unused-value -fno-omit-frame-pointer\
    ../src/scoundrel.cpp \
    ../src/log.cpp \
    ../src/application.cpp \
    ../src/renderer.cpp \
    ../src/camera.cpp \
    ../src/mouse.cpp \
    ../src/keyboard.cpp \
    ../src/audio.cpp \
    ../src/asset_manager.cpp \
    ../src/helpers.cpp \
    -x c ../libraries/glad/src/glad.c \
    $IncludeDirectories \
    -L../libraries/SDL-release-3.4.8 \
    -lSDL3 \
    -o scoundrel

popd > /dev/null
