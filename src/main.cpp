#include <SDL.h>
#include <iostream>
#include <SDL_image.h>
#include "game.h"
#define SDL_MAIN_HANDLED
#include "constants.h"
#include <Windows.h>

Game *game = nullptr;

int main(int argc, char* argv[]) {
    //console
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        freopen("CONIN$", "r", stdin);
    }
    //console end

    const int frameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;
    game = new Game();

    game->init(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FULLSCREEN);
    
    std::cout << "Console attached successfully!" << std::endl;
    if (!game->running()) {
        std::cerr << "Failed to initialize the game." << std::endl;
        return -1;
    }
    

    while(game->running()){
        frameStart = SDL_GetTicks();
        game->handleEvents();
        game->update();
        game->render();

        frameTime = SDL_GetTicks() - frameStart;

        if(frameDelay > frameTime){
            SDL_Delay(frameDelay - frameTime);
        }
    }

    game->clean();
    
    return 0;

}