#pragma once

#include <SDL.h>
#include <iostream>
#include <vector>


class AssetManager;
class ColliderComponent;

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
        static SDL_Event event;
       
        static bool isRunning;
        static SDL_Rect camera;
        static AssetManager* assets;

        enum groupLabels : std::size_t{
            groupMap,
            groupPlayers,
            groupColliders,
            groupProjectiles,
        };
        
    private:
        int count = 0;
        Uint32 lastShotTime = 0;
        SDL_Window *window;
        


};

