#!/bin/bash

g++ game.cpp -o app $(sdl2-config --cflags --libs) -lSDL2_ttf
