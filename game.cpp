#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>

typedef uint16_t u16;
typedef int32_t i32;

static constexpr size_t SCREEN_WIDTH = 800;
static constexpr size_t SCREEN_HEIGHT = 600;

static constexpr char FONT_PATH[] =
    "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";

/* ==== Game ==== */
typedef struct GameBoard {
    u16 width, height;

    u16 *data = nullptr;

    void init(u16 width, u16 height) {
        this->height = height;
        this->width = width;

        if (this->data != nullptr)
            free(data);

        this->data = (u16 *)calloc(height * width, sizeof(u16));
    }

    bool getCellValue(u16 x, u16 y, u16 &value) {
        if (x >= width || y >= height || data == nullptr)
            return false;

        value = this->data[y * width + x];
        return true;
    }

    bool setCellValue(u16 x, u16 y, u16 value) {
        if (x >= width || y >= height || data == nullptr)
            return false;

        this->data[y * width + x] = value;
        return true;
    }
} GameBoard;

typedef struct GameConfig {
    u16 width, height;
} GameConfig;

void initGame(GameConfig const &config, GameBoard &game) {
    game.init(config.width, config.height);
}

/* ==== Rendering ==== */
struct GameRenderer {
    u16 screen_width, screen_height;
    const char *window_name;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    u16 target_fps = 60;
};

void renderGame(GameBoard const &board, GameRenderer &game_renderer) {

    auto &renderer = game_renderer.renderer;
    // Actual render loop
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);

    SDL_RenderPresent(renderer);

    // Nothing to render yet
    SDL_Delay(1000 / game_renderer.target_fps); // ~60 FPS
}

i32 initRenderer(GameRenderer &config) {
    // 1. Initialize SDL (video subsystem only)
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF Init error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 2. Create window
    config.window = SDL_CreateWindow(
        config.window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config.screen_width, config.screen_height, SDL_WINDOW_SHOWN);

    if (!config.window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create renderer
    config.renderer =
        SDL_CreateRenderer(config.window, -1, SDL_RENDERER_ACCELERATED);
    if (!config.renderer) {
        std::cerr << "Renderer could not be created!" << std::endl
                  << "SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    return 0;
}

void cleanRenderer(GameRenderer &renderer) {
    SDL_DestroyRenderer(renderer.renderer);
    SDL_DestroyWindow(renderer.window);
    SDL_Quit();
}

/* ==== Input ==== */
struct GameStatus {
    bool running;
};

void handlePlayerInput(GameBoard const &board, GameRenderer &game_renderer,
                       GameStatus &game_status) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game_status.running = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_q)
                game_status.running = false;
        }
    }
}

/* ==== MAIN ==== */

GameStatus game_status;
static const char *APP_NAME = "2048 Renew";

GameBoard game;
const GameConfig DEFAULT_GAME_CONFIG = {4, 5};
GameRenderer game_renderer = {800, 800, APP_NAME, nullptr, nullptr};

int main(int argc, char *argv[]) {

    initGame(DEFAULT_GAME_CONFIG, game);

    if (initRenderer(game_renderer) != 0)
        return 1;

    game_status.running = true;

    while (game_status.running) {
        handlePlayerInput(game, game_renderer, game_status);
        renderGame(game, game_renderer);
    }

    cleanRenderer(game_renderer);

    return 0;
}
