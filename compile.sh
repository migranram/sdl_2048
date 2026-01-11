#!/bin/bash

g++ main.cpp -o app $(sdl2-config --cflags --libs) -lSDL2_ttf
