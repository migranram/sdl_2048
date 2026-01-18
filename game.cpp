#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cassert>
#include <iostream>
#include <stdlib.h>
#include <time.h>

typedef uint16_t u16;
typedef int32_t i32;
typedef uint8_t u8;
typedef int8_t i8;

static constexpr char FONT_PATH[] =
  "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";

/* ==== Game ==== */
typedef struct GameBoard
{
    u16 width, height;

    u16* data = nullptr;

    void init(u16 width, u16 height)
    {
        this->height = height;
        this->width  = width;

        if (this->data != nullptr)
            free(data);

        this->data = (u16*)calloc(height * width, sizeof(u16));
    }

    u16 getCellValue(u16 x, u16 y) const
    {
        assert(x < width || y < height || data != nullptr);

        return this->data[y * width + x];
    }

    void setCellValue(u16 x, u16 y, u16 value)
    {
        assert(x < width || y < height || data != nullptr);

        this->data[y * width + x] = value;
    }

    void shiftVertical(i8 sign, u16 col, u16 start = 0)
    {
        assert(sign == 1 || sign == -1);

        u16 init_val   = start;
        u16 target_val = (sign == 1) ? this->height - 1 : 0;

        for (u16 j = init_val; j != target_val; j += sign)
        {
            u16 next_val = this->getCellValue(col, j + sign);

            this->setCellValue(col, j, next_val);

            if (j == target_val - sign)
            {
                this->setCellValue(col, j + sign, 0);
            }
        }
    }

    void shiftHorizontal(i8 sign, u16 row, u16 start = 0)
    {
        assert(sign == 1 || sign == -1);

        u16 init_val   = start;
        u16 target_val = (sign == 1) ? this->width - 1 : 0;

        for (u16 i = init_val; i != target_val; i += sign)
        {
            u16 next_val = this->getCellValue(i + sign, row);

            this->setCellValue(i, row, next_val);

            if (i == target_val - sign)
            {
                this->setCellValue(i + sign, row, 0);
            }
        }
    }

    void moveVertical(i8 sign)
    {
        assert(sign == 1 || sign == -1);
        u16 init_val   = (sign == 1) ? 0 : this->height - 1;
        u16 target_val = (sign == 1) ? this->height - 1 : 0;

        for (u16 i = 0; i < this->width; i++)
        {
            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val = this->getCellValue(i, j);
                u16 dist     = 1;
                while (curr_val == 0 && dist++ < this->height)
                {
                    shiftVertical(sign, i, j);
                    curr_val = this->getCellValue(i, j);
                }
            }

            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val  = this->getCellValue(i, j);
                u16 other_val = this->getCellValue(i, j + sign);

                if (curr_val == other_val)
                {
                    this->setCellValue(i, j, curr_val * 2);
                    this->setCellValue(i, j + sign, 0);
                }
            }

            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val = this->getCellValue(i, j);
                u16 dist     = 1;
                while (curr_val == 0 && dist++ < this->height)
                {
                    shiftVertical(sign, i, j);
                    curr_val = this->getCellValue(i, j);
                }
            }
        }
    }

    void moveHorizontal(i8 sign)
    {
        assert(sign == 1 || sign == -1);
        u16 init_val   = (sign == 1) ? 0 : this->width - 1;
        u16 target_val = (sign == 1) ? this->width - 1 : 0;

        for (u16 i = 0; i < this->height; i++)
        {
            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val = this->getCellValue(j, i);
                u16 dist     = 1;
                while (curr_val == 0 && dist++ < this->width)
                {
                    shiftHorizontal(sign, i, j);
                    curr_val = this->getCellValue(j, i);
                }
            }

            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val  = this->getCellValue(j, i);
                u16 other_val = this->getCellValue(j + sign, i);

                if (curr_val == other_val)
                {
                    this->setCellValue(j, i, curr_val * 2);
                    this->setCellValue(j + sign, i, 0);
                }
            }

            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val = this->getCellValue(j, i);
                u16 dist     = 1;
                while (curr_val == 0 && dist++ < this->width)
                {
                    shiftHorizontal(sign, i, j);
                    curr_val = this->getCellValue(j, i);
                }
            }
        }
    }

    void moveUp()
    {
        moveVertical(1);
        addRandomValue();
    }
    void moveDown()
    {
        moveVertical(-1);
        addRandomValue();
    }
    void moveLeft()
    {
        moveHorizontal(1);
        addRandomValue();
    }
    void moveRight()
    {
        moveHorizontal(-1);
        addRandomValue();
    }

    u16 getEmptyCells() const
    {
        u16 count = 0;
        for (u16 i = 0; i < this->width; i++)
        {
            for (u16 j = 0; j < this->height; j++)
            {
                if (getCellValue(i, j) == 0)
                    count++;
            }
        }
        return count;
    }

    u16 getMaxValue() const
    {
        u16 max = 0;
        for (u16 i = 0; i < this->width; i++)
        {
            for (u16 j = 0; j < this->height; j++)
            {
                auto val = getCellValue(i, j);
                if (val > max)
                    max = val;
            }
        }
        return max;
    }

    bool checkBlock()
    {
        if (getEmptyCells() > 0)
            return false;

        return true;
    }

    void addRandomValue(u16 max_pow = 4)
    {
        const int r   = rand() % max_pow;
        const u16 val = std::pow(2, r);

        u16 x, y;
        do
        {
            x = rand() % this->width;
            y = rand() % this->height;
            if (getEmptyCells() == 0)
                return;
        } while (getCellValue(x, y) != 0);

        setCellValue(x, y, val);
    }
} GameBoard;

typedef struct GameConfig
{
    u16 width, height;
} GameConfig;

struct GameStatus
{
    bool running;
    bool can_play;
};

void initGame(GameConfig const& config, GameBoard& game)
{
    srand(time(NULL));
    game.init(config.width, config.height);
    game.addRandomValue(2);
}

/* ==== Rendering ==== */
struct GameRenderer
{
    u16 screen_width, screen_height;
    const char* window_name;
    SDL_Window* window     = nullptr;
    SDL_Renderer* renderer = nullptr;
    u16 target_fps         = 60;
    TTF_Font* font         = nullptr;
};

static const SDL_Color BACKGROUND_COLOR = {0xFF, 0xFF, 0xFF, 0xFF};

SDL_Color getCellColor(u16 value)
{
    switch (value)
    {
    case 0:
        return {200, 200, 200, 255}; // empty
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
        return {60, 58, 50, 255}; // overflow tiles
    }
}

int renderCell(u16 x,
               u16 y,
               u16 w,
               u16 h,
               u16 value,
               GameRenderer const& game_renderer)
{
    auto const& renderer    = game_renderer.renderer;
    SDL_Rect backgroundRect = {x, y, w, h};

    SDL_Texture* textTexture;
    {
        char value_txt[10] = "";
        if (value)
            sprintf(value_txt, "%d", value);
        SDL_Color textColor      = {0, 0, 0, 255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(
          game_renderer.font, value_txt, textColor);
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
    const auto color = getCellColor(value);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &backgroundRect);

    // Text
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

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
    SDL_SetRenderDrawColor(renderer, 0x66, 0xBB, 0xAA, 0xAA);
    SDL_RenderFillRect(renderer, &backgroundRect);

    // Text
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    return 0;
}

int renderGame(GameBoard const& board, GameRenderer& game_renderer, GameStatus& game_status)
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

            renderCell(posX, posY, cw - padding * 2, ch - padding * 2, board.getCellValue(i, j), game_renderer);
        }
    }

    // Render
    SDL_RenderPresent(renderer);

    // Wait for target FPS
    SDL_Delay(1000 / game_renderer.target_fps); // ~60 FPS

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

/* ==== Input ==== */

void handlePlayerInput(GameBoard& board, GameRenderer& game_renderer,
                       GameStatus& game_status)
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
            if (game_status.can_play == true)
            {
                if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
                    board.moveDown();
                if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
                    board.moveUp();
                if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
                    board.moveLeft();
                if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
                    board.moveRight();
            }
        }
    }
}

/* ==== MAIN ==== */

GameStatus game_status;
static const char* APP_NAME = "2048 Renew";

GameBoard game;
const GameConfig DEFAULT_GAME_CONFIG = {4, 5};
GameRenderer game_renderer           = {800, 800, APP_NAME, nullptr, nullptr};

int main(int argc, char* argv[])
{

    initGame(DEFAULT_GAME_CONFIG, game);

    if (initRenderer(game_renderer) != 0)
        return 1;

    game_status.running  = true;
    game_status.can_play = true;

    while (game_status.running)
    {
        handlePlayerInput(game, game_renderer, game_status);
        if (renderGame(game, game_renderer, game_status))
            game_status.running = false;
        if (game.checkBlock())
            game_status.can_play = false;
    }

    cleanRenderer(game_renderer);

    return 0;
}
