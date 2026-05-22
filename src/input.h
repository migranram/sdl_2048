/* ==== Input ==== */
#pragma once

#include <SDL2/SDL.h>

#include "gameboard.h"
#include "gamestatus.h"

void handlePlayerInput(GameBoard& board, GameStatus& game_status)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            game_status.running = false;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_q)
                game_status.running = false;
            if (game_status.can_play == true && game_status.phase == GamePhase::IDLE)
            {
                if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
                    board.moveDown();
                if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
                    board.moveUp();
                if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
                    board.moveLeft();
                if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
                    board.moveRight();

                game_status.phase = GamePhase::ANIMATION;
            }
        }
    }
}