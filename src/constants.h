#pragma once

// Window settings
const char* const WINDOW_TITLE = "Game";
const int WINDOW_POS_X = SDL_WINDOWPOS_CENTERED;
const int WINDOW_POS_Y = SDL_WINDOWPOS_CENTERED;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const bool WINDOW_FULLSCREEN = false;
const int FPS = 60;

//CHARACTER SETTINGS
const int CHAR_W = 64;
const int CHAR_H = 64;
const int CHAR_X = 400;
const int CHAR_Y = 300;
const int playerHealth = 100;
const int playerSpeed = 3;
const char* const playerSprites = "sprites/character/player_anims.png";

//ENEMY SETTINGS

//sprite
const char* const aligator1Sprite = "sprites/enemy/aligator1.png";
const char* const aligator2Sprite = "sprites/enemy/aligator2.png";
const char* const bear1Sprite = "sprites/enemy/bear1.png";
const char* const bear2Sprite = "sprites/enemy/bear2.png";
const char* const eliteSkeletonShieldSprite = "sprites/enemy/eliteskeleton_shield.png";
const char* const enemy1Sprite = "sprites/enemy/enemy1.png";
const char* const ina1Sprite = "sprites/enemy/ina1.png";
const char* const ina2Sprite = "sprites/enemy/ina2.png";
const char* const ina3Sprite = "sprites/enemy/ina3.png";
const char* const kfc1Sprite = "sprites/enemy/kfc1.png";
const char* const kfc2Sprite = "sprites/enemy/kfc2.png";
const char* const shrimp1Sprite = "sprites/enemy/shrimp1.png";
const char* const shrimp2Sprite = "sprites/enemy/shrimp2.png";
const char* const shrimp3Sprite = "sprites/enemy/shrimp3.png";
const char* const shrimp5Sprite = "sprites/enemy/shrimp5.png";
const char* const shrimpTankSprite = "sprites/enemy/shrimp_tank.png";
const char* const skeleton1Sprite = "sprites/enemy/skeleton1.png";
const char* const skeleton2Sprite = "sprites/enemy/skeleton2.png";
const char* const skeleton3Sprite = "sprites/enemy/skeleton3.png";
const char* const skeleton4Sprite = "sprites/enemy/skeleton4.png";
const char* const skeleton5Sprite = "sprites/enemy/skeleton5.png";
const char* const skeletonShieldSprite = "sprites/enemy/skeleton_shield.png";
const char* const zombieSprite = "sprites/enemy/zombie.png";

// Damage values
const int aligator1Damage = 10;
const int aligator2Damage = 10;
const int bear1Damage = 15;
const int bear2Damage = 15;
const int eliteSkeletonShieldDamage = 20;
const int enemy1Damage = 5;
const int ina1Damage = 8;
const int ina2Damage = 8;
const int ina3Damage = 8;
const int kfc1Damage = 12;
const int kfc2Damage = 12;
const int shrimp1Damage = 6;
const int shrimp2Damage = 6;
const int shrimp3Damage = 6;
const int shrimp5Damage = 6;
const int shrimpTankDamage = 8;
const int skeleton1Damage = 10;
const int skeleton2Damage = 10;
const int skeleton3Damage = 10;
const int skeleton4Damage = 10;
const int skeleton5Damage = 10;
const int skeletonShieldDamage = 15;
const int zombieDamage = 12;

// Health values
const int aligator1Health = 100;
const int aligator2Health = 100;
const int bear1Health = 150;
const int bear2Health = 150;
const int eliteSkeletonShieldHealth = 200;
// const int enemy1Health = 50;
const int ina1Health = 80;
const int ina2Health = 80;
const int ina3Health = 80;
const int kfc1Health = 120;
const int kfc2Health = 120;
const int shrimp1Health = 40;
const int shrimp2Health = 40;
const int shrimp3Health = 40;
const int shrimp5Health = 40;
const int shrimpTankHealth = 60;
const int skeleton1Health = 90;
const int skeleton2Health = 90;
const int skeleton3Health = 90;
const int skeleton4Health = 90;
const int skeleton5Health = 90;
const int skeletonShieldHealth = 120;
const int zombieHealth = 110;

// Experience values
const int aligator1Exp = 10;
const int aligator2Exp = 10;
const int bear1Exp = 20;
const int bear2Exp = 20;
const int eliteSkeletonShieldExp = 30;
// const int enemy1Exp = 5;
const int ina1Exp = 12;
const int ina2Exp = 12;
const int ina3Exp = 12;
const int kfc1Exp = 15;
const int kfc2Exp = 15;
const int skeleton1Exp = 14;
const int skeleton2Exp = 14;
const int skeleton3Exp = 14;
const int skeleton4Exp = 14;
const int skeleton5Exp = 14;
const int skeletonShieldExp = 18;
const int zombieExp = 16;

// Speed values for each enemy
const float zombieSpeed = 1.0f;
const float aligator1Speed = 1.0f;
const float aligator2Speed = 1.0f;
const float bear1Speed = 1.0f;
const float bear2Speed = 1.0f;
const float eliteSkeletonShieldSpeed = 1.0f;
// const float enemy1Speed = 1.5f;
const float ina1Speed = 1.0f;
const float ina2Speed = 1.0f;
const float ina3Speed = 1.0f;
const float kfc1Speed = 1.0f;
const float kfc2Speed = 1.0f;
const float shrimpTankSpeed = 1.0f;
const float skeleton1Speed = 1.0f;
const float skeleton2Speed = 1.0f;
const float skeleton3Speed = 1.0f;
const float skeleton4Speed = 1.0f;
const float skeleton5Speed = 1.0f;
const float skeletonShieldSpeed = 1.0f;


//MAP SETTINGS
const int TILE_SIZE = 32;
const int MAP_WIDTH = 48;
const int MAP_HEIGHT = 46;
const char* const MAP = "sprites/map/mapss.png";

