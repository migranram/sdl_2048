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

    void moveVertical(i8 sign)
    {
        assert(sign == 1 || sign == -1);
        u16 init_val   = (sign == 1) ? 0 : this->height - 1;
        u16 target_val = (sign == 1) ? this->height - 1 : 0;

        for (u16 i = 0; i < this->width; i++)
        {
            for (u16 j = init_val; j != target_val; j += sign)
            {
                u16 curr_val  = this->getCellValue(i, j);
                u16 other_val = this->getCellValue(i, j + sign);
                if (curr_val == other_val)
                {
                    this->setCellValue(i, j, curr_val * 2);
                    this->setCellValue(i, j + sign, 0);
                }
                else if (curr_val == 0)
                {
                    this->setCellValue(i, j, other_val);
                    this->setCellValue(i, j + sign, 0);
                }
            }
        }
    }

    void moveHorizontal(i8 sign)
    {
        assert(sign == 1 || sign == -1);
        u16 init_val   = (sign == 1) ? 0 : this->width - 1;
        u16 target_val = (sign == 1) ? this->width - 1 : 0;

        for (u16 j = 0; j < this->height; j++)
        {
            for (u16 i = init_val; i != target_val; i += sign)
            {
                u16 curr_val  = this->getCellValue(i, j);
                u16 other_val = this->getCellValue(i + sign, j);
                if (curr_val == other_val)
                {
                    this->setCellValue(i, j, curr_val * 2);
                    this->setCellValue(i + sign, j, 0);
                }
                else if (curr_val == 0)
                {
                    this->setCellValue(i, j, other_val);
                    this->setCellValue(i + sign, j, 0);
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

    void addRandomValue()
    {
        const int r   = rand() % 3;
        const u16 val = std::pow(2, r);

        u16 x, y;
        do
        {
            x = rand() % this->width;
            y = rand() % this->height;
        } while (getCellValue(x, y) != 0);

        setCellValue(x, y, val);
    }
} GameBoard;

typedef struct GameConfig
{
    u16 width, height;
} GameConfig;

void initGame(GameConfig const& config, GameBoard& game)
{
    srand(time(NULL));
    game.init(config.width, config.height);
    game.addRandomValue();
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
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
    SDL_RenderFillRect(renderer, &backgroundRect);

    // Text
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

    return 0;
}

int renderHeader(u16 x,
                 u16 y,
                 u16 w,
                 u16 h,
                 GameRenderer const& game_renderer)
{
    auto const& renderer    = game_renderer.renderer;
    SDL_Rect backgroundRect = {x, y, w, h};

    SDL_Texture* textTexture;
    {
        SDL_Color textColor      = {0, 0, 0, 255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(
          game_renderer.font, "2048 Header!", textColor);
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

int renderGame(GameBoard const& board, GameRenderer& game_renderer)
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

    renderHeader(padding, padding, game_renderer.screen_width - padding * 2, header_h - padding * 2, game_renderer);

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
struct GameStatus
{
    bool running;
};

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

    game_status.running = true;

    while (game_status.running)
    {
        handlePlayerInput(game, game_renderer, game_status);
        if (renderGame(game, game_renderer))
            game_status.running = false;
        ;
    }

    cleanRenderer(game_renderer);

    return 0;
}
