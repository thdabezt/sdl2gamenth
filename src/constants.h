#pragma once

// --- Window Settings ---
const char* const WINDOW_TITLE = "Monster Shooter";
const int WINDOW_POS_X = SDL_WINDOWPOS_CENTERED;
const int WINDOW_POS_Y = SDL_WINDOWPOS_CENTERED;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const bool WINDOW_FULLSCREEN = false;
const int FPS = 60;

// --- Character Settings ---
const int CHAR_W = 64;
const int CHAR_H = 64;
const int CHAR_X = 400;
const int CHAR_Y = 300;
const int playerHealth = 100;
const int playerSpeed = 3;
const char* const playerSprites = "sprites/character/player_anims.png";

// --- Enemy Settings ---
// Base Enemy Sprites
const char* const aligator1Sprite = "sprites/enemy/aligator1.png";
const char* const bear1Sprite = "sprites/enemy/bear1.png";
const char* const ina1Sprite = "sprites/enemy/ina1.png";
const char* const kfc1Sprite = "sprites/enemy/kfc1.png";
const char* const skeleton1Sprite = "sprites/enemy/skeleton1.png";
const char* const zombieSprite = "sprites/enemy/zombie.png";

// Upgraded Enemy Sprites
const char* const aligator2Sprite = "sprites/enemy/aligator2.png";
const char* const bear2Sprite = "sprites/enemy/bear2.png";
const char* const eliteSkeletonShieldSprite = "sprites/enemy/eliteskeleton_shield.png";
const char* const ina2Sprite = "sprites/enemy/ina2.png";
const char* const ina3Sprite = "sprites/enemy/ina3.png";
const char* const kfc2Sprite = "sprites/enemy/kfc2.png";
const char* const skeleton2Sprite = "sprites/enemy/skeleton2.png";
const char* const skeleton3Sprite = "sprites/enemy/skeleton3.png";
const char* const skeleton4Sprite = "sprites/enemy/skeleton4.png";
const char* const skeleton5Sprite = "sprites/enemy/skeleton5.png";
const char* const skeletonShieldSprite = "sprites/enemy/skeleton_shield.png";

// --- Base Enemy Stats ---
// Aligator
const int aligator1Health = 100; const int aligator1Damage = 40; const float aligator1Speed = 1.0f; const int aligator1Exp = 30;
const int aligator2Health = 150; const int aligator2Damage = 80; const float aligator2Speed = 1.1f; const int aligator2Exp = 70;

// Bear
const int bear1Health = 150; const int bear1Damage = 40; const float bear1Speed = 1.0f; const int bear1Exp = 30;
const int bear2Health = 220; const int bear2Damage = 80; const float bear2Speed = 1.0f; const int bear2Exp = 70;

// Ina
const int ina1Health = 80; const int ina1Damage = 30; const float ina1Speed = 1.7f; const int ina1Exp = 20;
const int ina2Health = 120; const int ina2Damage = 40; const float ina2Speed = 1.7f; const int ina2Exp = 30;
const int ina3Health = 180; const int ina3Damage = 60; const float ina3Speed = 1.7f; const int ina3Exp = 40;

// KFC
const int kfc1Health = 50; const int kfc1Damage = 20; const float kfc1Speed = 2.5f; const int kfc1Exp = 20;
const int kfc2Health = 100; const int kfc2Damage = 50; const float kfc2Speed = 2.5f; const int kfc2Exp = 30;

// Skeleton
const int skeleton1Health = 90; const int skeleton1Damage = 20; const float skeleton1Speed = 1.0f; const int skeleton1Exp = 18;
const int skeleton2Health = 130; const int skeleton2Damage = 30; const float skeleton2Speed = 1.1f; const int skeleton2Exp = 30;
const int skeleton3Health = 180; const int skeleton3Damage = 60; const float skeleton3Speed = 1.1f; const int skeleton3Exp = 50;
const int skeleton4Health = 240; const int skeleton4Damage = 65; const float skeleton4Speed = 1.2f; const int skeleton4Exp = 60;
const int skeleton5Health = 300; const int skeleton5Damage = 80; const float skeleton5Speed = 1.2f; const int skeleton5Exp = 80;

// Skeleton Shield Variants
const int skeletonShieldHealth = 200; const int skeletonShieldDamage = 10; const float skeletonShieldSpeed = 0.9f; const int skeletonShieldExp = 50;
const int eliteSkeletonShieldHealth = 400; const int eliteSkeletonShieldDamage = 20; const float eliteSkeletonShieldSpeed = 0.8f; const int eliteSkeletonShieldExp = 100;

// Zombie
const int zombieHealth = 100; const int zombieDamage = 10; const float zombieSpeed = 1.0f; const int zombieExp = 9;

// --- Boss Settings ---
const int BOSS_SPRITE_WIDTH = 110;
const int BOSS_SPRITE_HEIGHT = 110;
const int BOSS_HEALTH = 1000;
const float BOSS_SPEED = 3.0f;
const int BOSS_SLAM_DAMAGE = 35;
const int BOSS_PROJECTILE_DAMAGE = 15;
const float BOSS_KNOCKBACK_FORCE = 40.0f;
const int BOSS_PROJECTILE_SIZE = 46;
const float BOSS_PROJECTILE_SPEED = 2.0f;
const int BOSS_PROJECTILE_PIERCE = 5;

const char* const bossWalkSprite = "sprites/enemy/boss_walk.png";
const char* const bossChargeSprite = "sprites/enemy/boss_charge.png";
const char* const bossSlamSprite = "sprites/enemy/boss_slam.png";
const char* const bossProjectileSprite = "sprites/projectile/boss_proj.png";

// --- Map Settings ---
const int TILE_SIZE = 32;
const int MAP_WIDTH = 80;
const int MAP_HEIGHT = 50;
const char* const MAP = "sprites/map/officialmap.png";