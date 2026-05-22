/* ==== Rendering ==== */
#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>

#include "common.h"
#include "gameboard.h"
#include "gamestatus.h"

static constexpr char FONT_PATH[] =
  "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";

struct GameRenderer
{
    u16 screen_width, screen_height;
    const char* window_name;
    SDL_Window* window     = nullptr;
    SDL_Renderer* renderer = nullptr;
    u16 target_fps         = 60;
    TTF_Font* font         = nullptr;

    const u64 animation_duration = 150; // ms
    u64 current_animation_delta  = 0;
};

static const SDL_Color BACKGROUND_COLOR = {0xFF, 0xFF, 0xFF, 0xFF};

SDL_Color getCellColor(u16 value)
{
    switch (value)
    {
    case 0:
        return {200, 200, 200, 255}; // empty
    case 1:
        return {100, 100, 100, 255};
    case 2:
        return {238, 228, 218, 255};
    case 4:
        return {237, 224, 200, 255};
    case 8:
        return {242, 177, 121, 255};
    case 16:
        return {245, 149, 99, 255};
    case 32:
        return {246, 124, 95, 255};
    case 64:
        return {246, 94, 59, 255};
    case 128:
        return {237, 207, 114, 255};
    case 256:
        return {237, 204, 97, 255};
    case 512:
        return {237, 200, 80, 255};
    case 1024:
        return {237, 197, 63, 255};
    case 2048:
        return {237, 194, 46, 255};
    default:
        return {200, 0, 200, 255}; // overflow tiles
    }
}

inline u8 blendU8(u8 a, u8 b, double alpha)
{
    if (alpha > 1.0)
        alpha = 1.0;
    else if (alpha < 0.0)
        alpha = 0.0;
    return (u8)(a * (1. - alpha) + b * (alpha));
}

SDL_Color blendColors(SDL_Color c0, SDL_Color cf, double alpha)
{
    return {
      blendU8(c0.r, cf.r, alpha),
      blendU8(c0.g, cf.g, alpha),
      blendU8(c0.b, cf.b, alpha),
      blendU8(c0.a, cf.a, alpha)};
}

int renderCellLL(u16 x,
                 u16 y,
                 u16 w,
                 u16 h,
                 char* value_txt,
                 SDL_Color color,
                 GameRenderer const& game_renderer

)
{
    auto const& renderer    = game_renderer.renderer;
    SDL_Rect backgroundRect = {x, y, w, h};

    SDL_Texture* textTexture;
    {
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface* textSurface;
        if (value_txt != nullptr)
            textSurface = TTF_RenderText_Blended(
              game_renderer.font, value_txt, textColor);
        else
            textSurface = TTF_RenderText_Blended(
              game_renderer.font, " ", textColor);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
    }

    int textW, textH;
    SDL_QueryTexture(textTexture, nullptr, nullptr, &textW, &textH);

    // Make a rectangle to fit the text
    SDL_Rect textRect = {backgroundRect.x + (backgroundRect.w - textW) / 2,
                         backgroundRect.y + (backgroundRect.h - textH) / 2, textW,
                         textH};

    /* Rendering */
    // Square
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &backgroundRect);

    // Text
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    return 0;
}

int renderCell(u16 x,
               u16 y,
               u16 w,
               u16 h,
               u16 value,
               GameRenderer const& game_renderer)
{
    char value_txt[10] = "";
    if (value)
        sprintf(value_txt, "%d", value);

    const auto color = getCellColor(value);

    renderCellLL(x, y, w, h, value_txt, color, game_renderer);

    return 0;
}

int renderHeader(u16 x,
                 u16 y,
                 u16 w,
                 u16 h,
                 GameRenderer const& game_renderer,
                 GameStatus const& status,
                 GameBoard const& board)
{
    auto const& renderer    = game_renderer.renderer;
    SDL_Rect backgroundRect = {x, y, w, h};

    SDL_Texture* textTexture;
    {
        char text[100];

        if (status.can_play)
            sprintf(text, "%s", "Get to 2048!");
        else
            sprintf(text, "Finished! Score achieved: %d", board.getMaxValue());
        SDL_Color textColor      = {0, 0, 0, 255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(
          game_renderer.font, text, textColor);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
    }

    int textW, textH;
    SDL_QueryTexture(textTexture, nullptr, nullptr, &textW, &textH);

    // Make a rectangle to fit the text
    SDL_Rect textRect = {backgroundRect.x + (backgroundRect.w - textW) / 2,
                         backgroundRect.y + (backgroundRect.h - textH) / 2, textW,
                         textH};

    /* Rendering */
    // Square
    if (status.phase == GamePhase::ANIMATION)
        SDL_SetRenderDrawColor(renderer, 0xBB, 0xAA, 0x66, 0xAA);
    else
        SDL_SetRenderDrawColor(renderer, 0x66, 0xBB, 0xAA, 0xAA);
    SDL_RenderFillRect(renderer, &backgroundRect);

    // Text
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    return 0;
}

int renderGame(GameBoard const& board, GameRenderer& game_renderer, GameStatus& game_status, u64 delta)
{

    auto& renderer = game_renderer.renderer;
    // Actual render loop
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g,
                           BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
    SDL_RenderClear(renderer);

    const u16 padding  = 4; // px
    const u16 cw       = game_renderer.screen_width / board.width;
    const u16 header_h = game_renderer.screen_height * 0.2;
    const u16 ch       = (game_renderer.screen_height - header_h) / board.height;

    renderHeader(padding, padding, game_renderer.screen_width - padding * 2, header_h - padding * 2, game_renderer, game_status, board);

    for (u16 i = 0; i < board.width; i++)
    {
        for (u16 j = 0; j < board.height; j++)
        {
            const u16 posX = i * cw + padding;
            const u16 posY = header_h + j * ch + padding;

            u16 curr_val   = board.getCellValue(i, j);
            u16 target_val = board.getCellTargetValue(i, j);

            if (curr_val == target_val)
                renderCell(posX, posY, cw - padding * 2, ch - padding * 2, curr_val, game_renderer);
            else
            {
                const double alpha = (double)game_renderer.current_animation_delta / game_renderer.animation_duration;
                const auto color   = blendColors(getCellColor(curr_val), getCellColor(target_val), alpha);
                renderCellLL(posX, posY, cw - padding * 2, ch - padding * 2, nullptr, color, game_renderer);
            }
        }
    }

    // Render
    SDL_RenderPresent(renderer);

    return 0;
}

i32 initRenderer(GameRenderer& config)
{
    // 1. Initialize SDL (video subsystem only)
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1)
    {
        std::cerr << "TTF Init error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 2. Create window
    config.window = SDL_CreateWindow(
      config.window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      config.screen_width, config.screen_height, SDL_WINDOW_SHOWN);

    if (!config.window)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create renderer
    config.renderer =
      SDL_CreateRenderer(config.window, -1, SDL_RENDERER_ACCELERATED);
    if (!config.renderer)
    {
        std::cerr << "Renderer could not be created!" << std::endl
                  << "SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Text inside the object
    config.font = TTF_OpenFont(FONT_PATH, 24);
    if (!config.font)
    {
        std::cerr << "Error loading font from: " << FONT_PATH << std::endl;
        return 1;
    }

    return 0;
}

void cleanRenderer(GameRenderer& renderer)
{
    SDL_DestroyRenderer(renderer.renderer);
    SDL_DestroyWindow(renderer.window);
    SDL_Quit();
}
