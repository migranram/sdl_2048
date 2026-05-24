##!/bin/bash
set -e

PREFIX=""
if command -v bear >/dev/null 2>&1; then
    PREFIX="bear -- "
fi

mkdir -p out
${PREFIX}g++ src/game.cpp -o out/game $(sdl2-config --cflags --libs) -lSDL2_ttf
