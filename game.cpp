#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>

typedef uint16_t u16;
typedef int32_t i32;

static constexpr size_t SCREEN_WIDTH = 800;
static constexpr size_t SCREEN_HEIGHT = 600;

static constexpr char FONT_PATH[] =
    "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";

static constexpr size_t TARGET_FPS = 60;

struct GameBoard {
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
};

struct GameStatus {
    GameBoard board;
    i32 selected_cell;

    void init(u16 width, u16 height) {
        board.init(width, height);
        selected_cell = -1;
    }
} game_status;

struct GameRenderConfig{};

void render_status(GameStatus* status);


int main(int argc, char *argv[]) {

    game_status.init(4, 5);

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
    SDL_Window *window =
        SDL_CreateWindow("Hello SDL2", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created!" << std::endl
                  << "SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Simple object
    SDL_Rect exampleRect;
    exampleRect.w = std::min(SCREEN_HEIGHT, SCREEN_WIDTH) / 3;
    exampleRect.h = std::min(SCREEN_HEIGHT, SCREEN_WIDTH) / 2;

    exampleRect.x = SCREEN_WIDTH / 2 - exampleRect.w / 2;
    exampleRect.y = SCREEN_HEIGHT / 2 - exampleRect.h / 2;

    // Text inside the object
    TTF_Font *font = TTF_OpenFont(FONT_PATH, 24);
    if (!font) {
        std::cerr << "Error loading font from: " << FONT_PATH << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Texture *textTexture;
    {

        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface *textSurface =
            TTF_RenderText_Blended(font, "Hello World", textColor);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
    }

    int textW, textH;
    SDL_QueryTexture(textTexture, nullptr, nullptr, &textW, &textH);

    // Make a rectangle to fit the text
    SDL_Rect textRect = {exampleRect.x + (exampleRect.w - textW) / 2,
                         exampleRect.y + (exampleRect.h - textH) / 2, textW,
                         textH};

    // 3. Simple event loop (so the window stays open)
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_q)
                    running = false;
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    exampleRect.w += 10;
                    exampleRect.x -= 5;
                    textRect = {exampleRect.x + (exampleRect.w - textW) / 2,
                                exampleRect.y + (exampleRect.h - textH) / 2,
                                textW, textH};
                }
                if (event.key.keysym.sym == SDLK_LEFT) {
                    exampleRect.w -= 10;
                    exampleRect.x += 5;
                    textRect = {exampleRect.x + (exampleRect.w - textW) / 2,
                                exampleRect.y + (exampleRect.h - textH) / 2,
                                textW, textH};
                }
                if (event.key.keysym.sym == SDLK_UP) {
                    exampleRect.h += 10;
                    exampleRect.y -= 5;
                    textRect = {exampleRect.x + (exampleRect.w - textW) / 2,
                                exampleRect.y + (exampleRect.h - textH) / 2,
                                textW, textH};
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                    exampleRect.h -= 10;
                    exampleRect.y += 5;
                    textRect = {exampleRect.x + (exampleRect.w - textW) / 2,
                                exampleRect.y + (exampleRect.h - textH) / 2,
                                textW, textH};
                }
            }
        }
        // Actual render loop
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
        SDL_RenderFillRect(renderer, &exampleRect);

        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_RenderPresent(renderer);

        // Nothing to render yet
        SDL_Delay(1000 / TARGET_FPS); // ~60 FPS
    }

    // 4. Cleanup
    SDL_DestroyTexture(textTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
