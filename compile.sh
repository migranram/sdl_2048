#!/bin/bash
mkdir -p out
g++ src/game.cpp -o out/game $(sdl2-config --cflags --libs) -lSDL2_ttf
