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
// Base Enemy Sprites
const char* const aligator1Sprite = "sprites/enemy/aligator1.png";
const char* const bear1Sprite = "sprites/enemy/bear1.png";
const char* const ina1Sprite = "sprites/enemy/ina1.png";
const char* const kfc1Sprite = "sprites/enemy/kfc1.png";
const char* const skeleton1Sprite = "sprites/enemy/skeleton1.png";
const char* const zombieSprite = "sprites/enemy/zombie.png";
// Add others if they are base types that appear at level 5+
// const char* const enemy1Sprite = "sprites/enemy/enemy1.png"; // Example

// Upgraded Enemy Sprites (Ensure these files exist)
const char* const aligator2Sprite = "sprites/enemy/aligator2.png";
const char* const bear2Sprite = "sprites/enemy/bear2.png";
const char* const eliteSkeletonShieldSprite = "sprites/enemy/eliteskeleton_shield.png"; // Assuming this is an upgrade/special
const char* const ina2Sprite = "sprites/enemy/ina2.png";
const char* const ina3Sprite = "sprites/enemy/ina3.png"; // Assuming further upgrade
const char* const kfc2Sprite = "sprites/enemy/kfc2.png";
const char* const skeleton2Sprite = "sprites/enemy/skeleton2.png";
const char* const skeleton3Sprite = "sprites/enemy/skeleton3.png";
const char* const skeleton4Sprite = "sprites/enemy/skeleton4.png";
const char* const skeleton5Sprite = "sprites/enemy/skeleton5.png";
const char* const skeletonShieldSprite = "sprites/enemy/skeleton_shield.png"; // Assuming this is an upgrade/special

// --- BASE STATS (Define for ALL mentioned enemies) ---

// Aligator
const int aligator1Health = 100;
const int aligator1Damage = 10;
const float aligator1Speed = 1.0f;
const int aligator1Exp = 10;
const int aligator2Health = 150; // Example upgrade stats
const int aligator2Damage = 15;
const float aligator2Speed = 1.1f;
const int aligator2Exp = 15;

// Bear
const int bear1Health = 150;
const int bear1Damage = 15;
const float bear1Speed = 1.0f;
const int bear1Exp = 20;
const int bear2Health = 220; // Example upgrade stats
const int bear2Damage = 22;
const float bear2Speed = 1.0f;
const int bear2Exp = 30;

// Ina
const int ina1Health = 80;
const int ina1Damage = 8;
const float ina1Speed = 1.0f;
const int ina1Exp = 12;
const int ina2Health = 120; // Example upgrade stats
const int ina2Damage = 12;
const float ina2Speed = 1.1f;
const int ina2Exp = 18;
const int ina3Health = 180; // Example upgrade stats
const int ina3Damage = 18;
const float ina3Speed = 1.2f;
const int ina3Exp = 25;

// KFC
const int kfc1Health = 120;
const int kfc1Damage = 12;
const float kfc1Speed = 1.0f;
const int kfc1Exp = 15;
const int kfc2Health = 180; // Example upgrade stats
const int kfc2Damage = 18;
const float kfc2Speed = 1.0f;
const int kfc2Exp = 22;

// Skeleton
const int skeleton1Health = 90;
const int skeleton1Damage = 10;
const float skeleton1Speed = 1.0f;
const int skeleton1Exp = 14;
const int skeleton2Health = 130; // Example upgrade stats
const int skeleton2Damage = 14;
const float skeleton2Speed = 1.1f;
const int skeleton2Exp = 20;
// Add skeleton3, 4, 5 stats...
const int skeleton3Health = 180; const int skeleton3Damage = 18; const float skeleton3Speed = 1.1f; const int skeleton3Exp = 26;
const int skeleton4Health = 240; const int skeleton4Damage = 22; const float skeleton4Speed = 1.2f; const int skeleton4Exp = 32;
const int skeleton5Health = 300; const int skeleton5Damage = 26; const float skeleton5Speed = 1.2f; const int skeleton5Exp = 40;


// Skeleton Shield Variants (Treat as separate types or upgrades)
const int skeletonShieldHealth = 150; // Example
const int skeletonShieldDamage = 15;
const float skeletonShieldSpeed = 0.9f;
const int skeletonShieldExp = 18;
const int eliteSkeletonShieldHealth = 350; // Example
const int eliteSkeletonShieldDamage = 25;
const float eliteSkeletonShieldSpeed = 0.8f;
const int eliteSkeletonShieldExp = 45;


// Zombie
const int zombieHealth = 110;
const int zombieDamage = 12;
const float zombieSpeed = 1.0f;
const int zombieExp = 1;


//MAP SETTINGS
const int TILE_SIZE = 32;
const int MAP_WIDTH = 80;
const int MAP_HEIGHT = 50;
const char* const MAP = "sprites/map/officialmap.png";

