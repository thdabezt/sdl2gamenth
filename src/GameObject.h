#pragma once
#include "game.h"

class GameObject {
    public:
        GameObject(const char* texturesheet, int x, int y, int w, int h);  
        ~GameObject();
        
        void Update();
        void Render();
    private:
        int xpos;
        int ypos;
        int width;
        int height;
        SDL_Texture* objTexture;
        SDL_Rect srcRect, destRect;
        
};