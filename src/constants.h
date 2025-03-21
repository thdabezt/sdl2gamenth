#pragma once

// Window settings
const char* const WINDOW_TITLE = "Game";
const int WINDOW_POS_X = SDL_WINDOWPOS_CENTERED;
const int WINDOW_POS_Y = SDL_WINDOWPOS_CENTERED;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 640;
const bool WINDOW_FULLSCREEN = false;
const int FPS = 60;

//CHARACTER SETTINGS
const int CHAR_W = 64;
const int CHAR_H = 64;
const int CHAR_X = 400;
const int CHAR_Y = 300;
const char* const playerSprites = "sprites/character/ametest.png";

//ENEMY SETTINGS
const int E1_W = 64;
const int E1_H = 64;
const int E1_X = 300;
const int E1_Y = 0;
const char* const enemy1Sprites = "sprites/enemy/enemy1.png";
