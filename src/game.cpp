#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>

/* ======================================== */

#include "common.h"
#include "gameboard.h"
#include "input.h"
#include "render.h"

/* ======================================== */

void initGame(GameConfig const& config, GameBoard& game)
{
    srand(time(NULL));
    game.init(config.width, config.height);
    game.addRandomValue(1, 2);
    game.copy_buffers(false);
}

GameStatus game_status;
static const char* APP_NAME = "2048 Renew";

GameBoard game;
const GameConfig DEFAULT_GAME_CONFIG = {4, 5};
GameRenderer game_renderer           = {
  800,
  800,
  APP_NAME,
  nullptr,
  nullptr,
  60,
  nullptr,
  150,
};

/* ======================================== */

/* ============ MAIN ============ */

inline u64 getTimeNow()
{
    return SDL_GetTicks64();
}

int main(int argc, char* argv[])
{

    initGame(DEFAULT_GAME_CONFIG, game);

    if (initRenderer(game_renderer) != 0)
        return 1;

    game_status.running  = true;
    game_status.can_play = true;

    const u64 target_ms = 1000 / game_renderer.target_fps;

    /// Timing
    u64 t_frame_ini, t_frame_final, delta;
    u64 t_last_frame_render = getTimeNow();
    u64 t_this_frame_render = t_last_frame_render;
    u64 frame_delta;

    /// Loop
    while (game_status.running)
    {
        t_frame_ini = getTimeNow();

        //-- Input & Update status
        handlePlayerInput(game, game_status); // Update the swap buffer

        //-- Render game
        t_this_frame_render = getTimeNow();
        frame_delta         = t_this_frame_render - t_last_frame_render;
        if (renderGame(game, game_renderer, game_status, frame_delta))
            game_status.running = false;

        if (game_status.phase == GamePhase::ANIMATION)
        {
            game_renderer.current_animation_delta += frame_delta;
            if (game_renderer.current_animation_delta >= game_renderer.animation_duration)
            {
                game_renderer.current_animation_delta = 0;
                game_status.phase                     = GamePhase::IDLE;
                game.copy_buffers(false);
            }
        }
        t_last_frame_render = t_this_frame_render;

        //-- Check for termination
        if (game.getMaxValue() == 2048)
            game_status.can_play = false;
        if (game.checkBlock())
            game_status.can_play = false;

        //-- Wait for next frame
        t_frame_final = getTimeNow();
        delta         = t_frame_final - t_frame_ini;

        if (delta >= target_ms)
            printf("[WARN] Loop took more (%lu) than frame budget (%lu)!!!\n", delta, target_ms);
        else
            SDL_Delay(target_ms - delta);
    }

    cleanRenderer(game_renderer);

    return 0;
}
