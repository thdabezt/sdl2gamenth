#pragma once

#include <SDL.h>

class Game {
    public:
        Game();

        ~Game();
        void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);

        void handleEvents();

        void update();

        void render();

        void clean();

        bool running(){
            return isRunning;
        }
        void setRunning(bool running) { isRunning = running; }
        static SDL_Renderer *renderer;

    private:
        int count = 0;
        bool isRunning;
        SDL_Window *window;
        


};

