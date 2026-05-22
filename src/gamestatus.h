#pragma once

#include "common.h"

typedef struct GameConfig
{
    u16 width, height;
} GameConfig;

typedef enum
{
    IDLE      = 0, // Waiting for user input
    ANIMATION = 1, // Animating transitions, no input accepted
} GamePhase;

struct GameStatus
{
    bool running      = true;
    bool can_play     = true;
    GamePhase phase = GamePhase::IDLE;
};