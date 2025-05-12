#include <iostream>
#include <raylib.h>
#include <cmath>
#include <vector>

using namespace std;

// ========================= GLOBAL STATE =========================
// Scarfy (player) state

enum GameScreen { MENU, GAMEPLAY, GAMEOVER };
GameScreen currentScreen = MENU;


Texture2D scarfyTexture;
Rectangle scarfyFrame;
Vector2 scarfyPosition;
Vector2 scarfyVelocity;
Sound walkSound;
Sound landingSound;
const float gravity = 1.0f;
const float scarfySpeed = 10.0f;
const float jumpStrength = -25.0f;
bool isJumping = false;
float groundY;
int scarfyWidth;
int frameIndex = 0;
int frameDelay = 5;
int frameCounter = 0;
unsigned int numberOfFrames = 6;

// Game state
Texture2D initialBg, bg2, bg3, bg4, bg5, bg6;
Texture2D menuBackground, menuLogo;
Texture2D gameOverBg, gameOverImage, playAgainBtn, scoreCardBtn;
Texture2D coinTexture, deathTexture, stoneTexture;
Texture2D heartTexture, treasureTexture;
Texture2D playButtonImage;
Rectangle playButtonBounds;

Sound coinSound, hurtSound;
unsigned int level = 1;
unsigned int score = 0;
int health = 100; // Changed from unsigned int to int for proper health check

// Entity states

struct Entity {
    Texture2D texture;
    Vector2 position;
    float speed;
    bool active;
    float damageCooldown;
    float cooldownTime;
};

vector<Entity> coins;
vector<Entity> deaths;
vector<Entity> stones;
vector<Entity> hearts;
vector<Entity> chests;

int maxCoins = 5;
int maxDeaths = 1;
int maxStones = 3;
int maxHearts = 1;
int maxChests = 1;

// Add these variables at the top with other global variables
int treasuresCollectedInLevel = 0;
int heartsCollectedInLevel = 0;
const int MAX_TREASURES_PER_LEVEL = 1;

// Add to global variables section
Texture2D dangerTexture;
Music backgroundMusic;
Sound gameOverSound;
float normalMusicPitch = 1.0f;
float slowMusicPitch = 0.8f;  // Changed from 0.8f to 0.6f for a more serious tone in level 6


// ========================= GAME INITIALIZATION FUNCTIONS =========================
void loadTextures() {

    // Load Menu textures
    menuBackground = LoadTexture("./resources/images/bg-1.jpg");
    menuLogo = LoadTexture("./resources/images/logo.png");
    playButtonImage = LoadTexture("./resources/images/play-button.png");

    // Load Game Over textures
    gameOverBg = LoadTexture("./resources/images/bg-6.jpg");
    gameOverImage = LoadTexture("./resources/images/game-over.png");
    playAgainBtn = LoadTexture("./resources/images/play-again.png");
    scoreCardBtn = LoadTexture("./resources/images/score-card.png");


    // Load background textures
    initialBg = LoadTexture("./resources/images/bg-1.jpg");
    bg2 = LoadTexture("./resources/images/bg-2.jpg");
    bg3 = LoadTexture("./resources/images/bg-3.jpg");
    bg4 = LoadTexture("./resources/images/bg-4.jpg");
    bg5 = LoadTexture("./resources/images/bg-5.jpg");
    bg6 = LoadTexture("./resources/images/bg-6.jpg");

     // Player texture
    scarfyTexture = LoadTexture("./resources/images/scarfy.png");

    // Entity textures
    coinTexture = LoadTexture("./resources/images/coin.png");
    deathTexture = LoadTexture("./resources/images/death.png");
    stoneTexture = LoadTexture("./resources/images/stone.png");
    heartTexture = LoadTexture("./resources/images/heart.png");
    treasureTexture = LoadTexture("./resources/images/treasure.png");

    // Add to loadTextures() function
    dangerTexture = LoadTexture("./resources/images/danger.png");
}

void loadSounds() {
    walkSound = LoadSound("./resources/sounds/Footstep-A.mp3");
    landingSound = LoadSound("./resources/sounds/Footstep-B.mp3");
    coinSound = LoadSound("./resources/sounds/coin.mp3");
    hurtSound = LoadSound("./resources/sounds/death.mp3");
    gameOverSound = LoadSound("./resources/sounds/game-over.mp3");
    backgroundMusic = LoadMusicStream("./resources/sounds/bg-music.mp3");
    SetMusicVolume(backgroundMusic, 0.5f);  // Set music volume to 50%
}

// ========================= SCARFY (PLAYER) FUNCTIONS =========================
void initScarfy() {
    scarfyWidth = scarfyTexture.width / numberOfFrames;
    scarfyFrame = {0, 0, (float)scarfyWidth, (float)scarfyTexture.height};
    groundY = (3 * GetScreenHeight()) / 4 - 50;
    scarfyPosition = {GetScreenWidth() / 2 - scarfyWidth / 2, groundY};
    scarfyVelocity = {0.0f, 0.0f};
}

void drawScarfy() {
    DrawTextureRec(scarfyTexture, scarfyFrame, scarfyPosition, WHITE);
}

void checkGroundCollision() {
    if (scarfyPosition.y >= groundY) {
        scarfyPosition.y = groundY;
        scarfyVelocity.y = 0.0f;
        isJumping = false;
        PlaySound(landingSound);
    }
}

void updateScarfyFrame() {
    frameCounter++;
    if (frameCounter >= frameDelay) {
        frameCounter = 0;
        frameIndex = (frameIndex + 1) % numberOfFrames;
        scarfyFrame.x = (float)(frameIndex * scarfyWidth);
        if (frameIndex == 1 || frameIndex == 4) PlaySound(walkSound);
    }
}

void updateScarfy() {
    // Horizontal movement
    if (IsKeyDown(KEY_RIGHT)) {
        scarfyVelocity.x = scarfySpeed;
        // Check right boundary
        if (scarfyPosition.x + scarfyWidth < GetScreenWidth()) {
            scarfyPosition.x += scarfyVelocity.x;
        }
        if (scarfyFrame.width < 0) scarfyFrame.width = -scarfyFrame.width;
        updateScarfyFrame();
    } 
    else if (IsKeyDown(KEY_LEFT)) {
        scarfyVelocity.x = scarfySpeed;
        // Check left boundary
        if (scarfyPosition.x > 0) {
            scarfyPosition.x -= scarfyVelocity.x;
        }
        if (scarfyFrame.width > 0) scarfyFrame.width = -scarfyFrame.width;
        updateScarfyFrame();
    }

    // Jump
    if (IsKeyDown(KEY_UP) && !isJumping) {
        scarfyVelocity.x = 0;
        scarfyVelocity.y = jumpStrength;
        frameIndex = 3; // Jump frame
        scarfyFrame.x = (float)(frameIndex * scarfyWidth);
        isJumping = true;
    }

    // Gravity
    scarfyVelocity.y += gravity;
    scarfyPosition.y += scarfyVelocity.y;

    // Ground collision check
    checkGroundCollision();

    // Ensure Scarfy stays within screen bounds
    if (scarfyPosition.x < 0) {
        scarfyPosition.x = 0;
    }
    if (scarfyPosition.x + scarfyWidth > GetScreenWidth()) {
        scarfyPosition.x = GetScreenWidth() - scarfyWidth;
    }
}

// ========================= ENTITY FUNCTIONS =========================
Entity createEntity(Texture2D tex, float x, float y, float speed) {
    Entity entity;
    entity.texture = tex;
    entity.position = {x, y};
    entity.speed = speed;
    entity.active = true;
    entity.damageCooldown = 0.0f;
    entity.cooldownTime = 1.0f;
    return entity;
}

void updateEntity(Entity& entity, bool horizontal = true) {
    if (entity.active) {
        if (horizontal) {
            entity.position.x += entity.speed;
            // For entities moving left (negative speed), check if they're off screen to the left
            if (entity.speed < 0 && entity.position.x < -100) {
                entity.active = false;
            }
            // For entities moving right (positive speed), check if they're off screen to the right
            else if (entity.speed > 0 && entity.position.x > GetScreenWidth()) {
                entity.active = false;
            }
        } else {
            entity.position.y += entity.speed;
            if (entity.position.y > GetScreenHeight()) {
                entity.active = false;
            }
        }
    }
    if (entity.damageCooldown > 0.0f) entity.damageCooldown -= GetFrameTime();
}

void drawEntity(const Entity& entity) {
    if (entity.active) {
        DrawTexture(entity.texture, entity.position.x, entity.position.y, WHITE);
    }
}

Rectangle getEntityCollisionRect(const Entity& entity) {
    return {entity.position.x, entity.position.y, (float)entity.texture.width, (float)entity.texture.height};
}



Entity createCoin() {
    float screenHeight = GetScreenHeight();
    float y = GetRandomValue(screenHeight / 2 - 50, screenHeight / 2 + 50);
    return createEntity(coinTexture, GetRandomValue(-500, -100), y, 8.0f);
}

Entity createDeath() {
    float screenHeight = GetScreenHeight();
    float y = GetRandomValue(screenHeight / 2, screenHeight / 2 + 270);
    return createEntity(deathTexture, GetRandomValue(-800, -100), y, 8.0f);
}

Entity createStone() {
    float screenWidth = GetScreenWidth();
    float x = GetRandomValue(screenWidth / 4, screenWidth / 4 * 3);
    return createEntity(stoneTexture, x, GetRandomValue(0, -300), 8.0f);
}

Entity createHeart() {
    float screenHeight = GetScreenHeight();
    float y = GetRandomValue(screenHeight / 2 - 50, screenHeight / 2 + 50);
    return createEntity(heartTexture, GetScreenWidth() + 100, y, -3.0f);  // Reduced speed from -4.0f to -3.0f
}

Entity createTreasure() {
    float screenHeight = GetScreenHeight();
    float y = GetRandomValue(screenHeight / 2 + 100, screenHeight / 2 + 200);  // Lower y position range
    return createEntity(treasureTexture, GetScreenWidth() + 100, y, -3.0f);  // Reduced speed from -4.0f to -3.0f
}

void initEntities() {
    // Initialize coins
    coins.clear();
    for (int i = 0; i < maxCoins; i++) {
        coins.push_back(createCoin());
    }

    // Initialize deaths
    deaths.clear();
    for (int i = 0; i < maxDeaths; i++) {
        deaths.push_back(createDeath());
    }

    // Initialize stones
    stones.clear();
    for (int i = 0; i < maxStones; i++) {
        stones.push_back(createStone());
    }

    // Initialize hearts
    hearts.clear();
    for (int i = 0; i < maxHearts; i++) {
        hearts.push_back(createHeart());
    }

    // Initialize treasures
    chests.clear();
    for (int i = 0; i < maxChests; i++) {
        chests.push_back(createTreasure());
    }
}


// ========================= GAME UPDATE FUNCTIONS =========================
void updateCoins() {
    for (int i = 0; i < coins.size(); i++) {
        updateEntity(coins[i]);

        Rectangle coinRect = getEntityCollisionRect(coins[i]);
        Rectangle scarfyRect = {
            scarfyPosition.x,
            scarfyPosition.y,
            (float)scarfyWidth,
            (float)scarfyTexture.height
        };

        if (coins[i].active && CheckCollisionRecs(coinRect, scarfyRect)) {
            PlaySound(coinSound);
            coins[i].active = false;
            score++;
        }

        if (!coins[i].active || coins[i].position.x > GetScreenWidth()) {
            coins[i] = createCoin();
        }
    }
}

void updateDeaths() {
    for (int i = 0; i < deaths.size(); i++) {
        updateEntity(deaths[i]);

        Rectangle deathRect = getEntityCollisionRect(deaths[i]);
        Rectangle scarfyRect = {
            scarfyPosition.x,
            scarfyPosition.y,
            (float)scarfyWidth,
            (float)scarfyTexture.height
        };

        if (deaths[i].active && CheckCollisionRecs(deathRect, scarfyRect)) {
            if (deaths[i].damageCooldown <= 0.0f) {
                PlaySound(hurtSound);
                health -= 7;
                deaths[i].damageCooldown = deaths[i].cooldownTime;
                
                // Check if health is depleted
                if (health <= 0) {
                    health = 0; // Ensure health doesn't go negative
                    currentScreen = GAMEOVER;
                }
            }
        }

        if (!deaths[i].active || deaths[i].position.x > GetScreenWidth()) {
            deaths[i] = createDeath();
        }
    }
}

void updateStones() {
    for (int i = 0; i < stones.size(); i++) {
        updateEntity(stones[i], false); // false means vertical movement

        Rectangle stoneRect = getEntityCollisionRect(stones[i]);
        Rectangle scarfyRect = {
            scarfyPosition.x,
            scarfyPosition.y,
            (float)scarfyWidth,
            (float)scarfyTexture.height
        };

        if (stones[i].active && CheckCollisionRecs(stoneRect, scarfyRect)) {
            if (stones[i].damageCooldown <= 0.0f) {
                PlaySound(hurtSound);
                health -= 3;
                stones[i].damageCooldown = stones[i].cooldownTime;
                
                // Check if health is depleted
                if (health <= 0) {
                    health = 0; // Ensure health doesn't go negative
                    currentScreen = GAMEOVER;
                }
            }
        }

        if (!stones[i].active || stones[i].position.y > GetScreenHeight()) {
            stones[i] = createStone();
        }
    }
}

void updateHearts() {
    for (int i = 0; i < hearts.size(); i++) {
        updateEntity(hearts[i], true);

        Rectangle heartRect = getEntityCollisionRect(hearts[i]);
        Rectangle scarfyRect = {
            scarfyPosition.x,
            scarfyPosition.y,
            (float)scarfyWidth,
            (float)scarfyTexture.height
        };

        if (hearts[i].active && CheckCollisionRecs(heartRect, scarfyRect)) {
            if (hearts[i].damageCooldown <= 0.0f) {
                PlaySound(coinSound);
                health += 10;
                hearts[i].active = false;
                hearts[i].damageCooldown = hearts[i].cooldownTime;
                heartsCollectedInLevel++;
                
                if (health > 100) {
                    health = 100;
                }
            }
        }

        // Remove the heart if it's gone off screen or collected
        if (!hearts[i].active || hearts[i].position.x < -100) {
            hearts.erase(hearts.begin() + i);
            i--;  // Adjust index since we removed an element
        }
    }
}

void updateTreasures() {
    for (int i = 0; i < chests.size(); i++) {
        updateEntity(chests[i], true);

        Rectangle chestRect = getEntityCollisionRect(chests[i]);
        Rectangle scarfyRect = {
            scarfyPosition.x,
            scarfyPosition.y,
            (float)scarfyWidth,
            (float)scarfyTexture.height
        };

        if (chests[i].active && CheckCollisionRecs(chestRect, scarfyRect)) {
            if (chests[i].damageCooldown <= 0.0f) {
                PlaySound(coinSound);
                score += 10;
                chests[i].active = false;
                chests[i].damageCooldown = chests[i].cooldownTime;
                treasuresCollectedInLevel++;
            }
        }

        if (!chests[i].active || chests[i].position.x < -100) {
            chests.erase(chests.begin() + i);
            i--;
        }
    }
}

// ========================= LEVEL PROGRESSION FUNCTIONS =========================

void updateLevelProgression() {
    // Reset counters when level changes
    static int lastLevel = 1;
    if (lastLevel != level) {
        treasuresCollectedInLevel = 0;
        heartsCollectedInLevel = 0;
        // Clear entities when changing levels
        chests.clear();
        hearts.clear();
        lastLevel = level;
    }

    // Resize vectors if needed based on level progression
    if (score > 20 && score <= 40) {
        level = 2;
        maxDeaths = 2;
        maxStones = 3;
        maxHearts = 0;
        maxChests = 0;
        hearts.clear();
        chests.clear();
    } else if (score > 40 && score <= 60) {
        level = 3;
        maxDeaths = 3;
        maxStones = 4;
        maxHearts = 0;
        maxChests = (treasuresCollectedInLevel == 0) ? 1 : 0;
        hearts.clear();
    } else if (score > 60 && score <= 75) {
        level = 4;
        maxDeaths = 3;
        maxStones = 4;
        maxHearts = (heartsCollectedInLevel == 0) ? 1 : 0;  // One heart in level 4
        maxChests = 0;
        chests.clear();
    } else if (score > 75 && score <= 90) {
        level = 5;
        maxDeaths = 4;
        maxStones = 5;
        maxHearts = 0;
        maxChests = 0;
        hearts.clear();
        chests.clear();
    } else if (score > 90) {
        level = 6;
        maxDeaths = 5;
        maxStones = 5;
        maxHearts = (heartsCollectedInLevel == 0) ? 1 : 0;  // One heart in level 6
        maxChests = (treasuresCollectedInLevel == 0) ? 1 : 0;
    }

    // Ensure we have the right number of entities
    while (deaths.size() < maxDeaths) {
        deaths.push_back(createDeath());
    }
    
    while (stones.size() < maxStones) {
        stones.push_back(createStone());
    }

    // Only create new heart if we have none and haven't collected any in this level
    // AND we're in level 4 or 6
    if (hearts.size() == 0 && maxHearts > 0 && heartsCollectedInLevel == 0 && (level == 4 || level == 6)) {
        hearts.push_back(createHeart());
    }

    // Only create new treasure if we have none and haven't collected any in this level
    // AND we're in level 3 or 6
    if (chests.size() == 0 && maxChests > 0 && treasuresCollectedInLevel == 0 && (level == 3 || level == 6)) {
        chests.push_back(createTreasure());
    }
}

void updateGame() {
    updateScarfy();
    updateCoins();
    updateDeaths();
    updateStones();
    updateHearts();
    updateTreasures();
    updateLevelProgression();
    
    // Check health status
    if (health <= 0) {
        health = 0; // Ensure health doesn't go negative
        currentScreen = GAMEOVER;
    }
}


// ========================= DRAWING FUNCTIONS =========================
void drawBackground() {
    Texture2D currentBg;
    
    switch (level) {
        case 1: currentBg = initialBg; break;
        case 2: currentBg = bg2; break;
        case 3: currentBg = bg3; break;
        case 4: currentBg = bg4; break;
        case 5: currentBg = bg5; break;
        case 6: currentBg = bg6; break;
        default: currentBg = initialBg;
    }
    
    // BADA KR RAHA HAI
    float bgScaleX = (float)GetScreenWidth() / currentBg.width;
    float bgScaleY = (float)GetScreenHeight() / currentBg.height;
    float scale = fmax(bgScaleX, bgScaleY);

    // CENTER KR RAHA HAI
    float offsetX = (GetScreenWidth() - currentBg.width * scale) / 2.0f;
    float offsetY = (GetScreenHeight() - currentBg.height * scale) / 2.0f;

    // DRAW KR RAHA HAI
    DrawTextureEx(currentBg, {offsetX, offsetY}, 0.0f, scale, WHITE);
}

void drawEntities() {
    for (const auto& coin : coins) {
        drawEntity(coin);
    }
    
    for (const auto& death : deaths) {
        drawEntity(death);
    }
    
    for (const auto& stone : stones) {
        drawEntity(stone);
    }

    for (const auto& heart : hearts) {
        if (heart.active) {
            drawEntity(heart);
        }
    }

    for (const auto& chest : chests) {
        if (chest.active) {
            drawEntity(chest);
        }
    }
}

void drawUI() {
    if (level == 6) {
        DrawText("Final Level", GetScreenWidth() / 2, 50, 60, BLACK);
    } else {
        DrawText(TextFormat("Level: %i", level), GetScreenWidth() / 2, 50, 60, BLACK);
    }
    DrawText(TextFormat("Score: %i", score), 40, 20, 60, WHITE);
    DrawText(TextFormat("Health: %i", health), 40, 70, 60, WHITE);
}

void drawMenu(Texture2D bg, Texture2D logo, Texture2D playButtonImage) {

    // FOR SCALING THE IMAGE OR MAKING IT BIGGER
    float bgScaleX = (float)GetScreenWidth() / bg.width;
    float bgScaleY = (float)GetScreenHeight() / bg.height;
    float scale = fmax(bgScaleX, bgScaleY);

    // FOR CENTRING THE IMAGE
    float offsetX = (GetScreenWidth() - bg.width * scale) / 2.0f;
    float offsetY = (GetScreenHeight() - bg.height * scale) / 2.0f;


    // MAIN THING IS SCALING
    DrawTextureEx(bg, {offsetX, offsetY}, 0.0f, scale, WHITE);

    // Center logo
    int logoX = (GetScreenWidth() - logo.width) / 2;
    int logoY = GetScreenHeight() / 4;
    DrawTexture(logo, logoX, logoY, WHITE);

    // Calculate Play Button position
    int btnX = (GetScreenWidth() - playButtonImage.width) / 2;
    int btnY = GetScreenHeight() / 2 + 200;
    Rectangle buttonBounds = { (float)btnX, (float)btnY, (float)playButtonImage.width, (float)playButtonImage.height };
    playButtonBounds = buttonBounds; // Store for click detection

    // Draw Play Button Image
    DrawTexture(playButtonImage, btnX, btnY, WHITE);

    // Handle click
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), playButtonBounds)) {
        currentScreen = GAMEPLAY;
    }
}

void drawGameOverMenu(Texture2D gameOverBg, Texture2D gameOverImage, Texture2D playAgainBtn, Texture2D scoreCardBtn) {
    // Play game over sound when menu is first shown
    static bool soundPlayed = false;
    if (!soundPlayed) {
        StopMusicStream(backgroundMusic);
        PlaySound(gameOverSound);
        soundPlayed = true;
    }

    // Scale background to fit screen
    float bgScaleX = (float)GetScreenWidth() / gameOverBg.width;
    float bgScaleY = (float)GetScreenHeight() / gameOverBg.height;
    float scale = fmax(bgScaleX, bgScaleY);

    float offsetX = (GetScreenWidth() - gameOverBg.width * scale) / 2.0f;
    float offsetY = (GetScreenHeight() - gameOverBg.height * scale) / 2.0f;

    DrawTextureEx(gameOverBg, {offsetX, offsetY}, 0.0f, scale, WHITE);

    // Draw Game Over logo in center top
    int logoX = (GetScreenWidth() - gameOverImage.width) / 2;
    int logoY = GetScreenHeight() / 4;
    DrawTexture(gameOverImage, logoX, logoY, WHITE);

    // --- Play Again Button ---
    int playBtnX = (GetScreenWidth() - playAgainBtn.width) / 2;
    int playBtnY = logoY + gameOverImage.height + 50; // Reduced gap from 100 to 50
    Rectangle playBtnBounds = { (float)playBtnX, (float)playBtnY, (float)playAgainBtn.width, (float)playAgainBtn.height };
    DrawTexture(playAgainBtn, playBtnX, playBtnY, WHITE);

    // --- Score Card Button ---
    int scoreBtnX = (GetScreenWidth() - scoreCardBtn.width) / 2;
    int scoreBtnY = playBtnY + playAgainBtn.height + 20;
    Rectangle scoreBtnBounds = { (float)scoreBtnX, (float)scoreBtnY, (float)scoreCardBtn.width, (float)scoreCardBtn.height };
    DrawTexture(scoreCardBtn, scoreBtnX, scoreBtnY, WHITE);
    
    // Draw score on the score card button
    int scoreTxtX = scoreBtnX + scoreCardBtn.width/2 - 120;
    int scoreTxtY = scoreBtnY + scoreCardBtn.height/2 - 15;
    DrawText(TextFormat("FINAL SCORE: %d", score), scoreTxtX, scoreTxtY, 25, BLACK);

    // Handle Play Again click
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), playBtnBounds)) {
        currentScreen = GAMEPLAY;
        // Reset game state
        score = 0;
        level = 1;
        health = 100;
        maxDeaths = 1;
        maxStones = 3;
        maxHearts = 0;
        maxChests = 0;
        treasuresCollectedInLevel = 0;
        heartsCollectedInLevel = 0;
        
        // Clear all entities
        coins.clear();
        deaths.clear();
        stones.clear();
        hearts.clear();
        chests.clear();
        
        // Reset Scarfy
        initScarfy();
        
        // Initialize new entities
        initEntities();

        // Restart background music
        PlayMusicStream(backgroundMusic);
        soundPlayed = false;  // Reset sound flag for next game over
    }
}

void drawGame() {
    drawBackground();
    
    // Draw danger image if health is low
    if (health < 50) {
        float dangerScale = 1.5f;
        float dangerX = 50.0f; // Distance from left border
        float dangerY = groundY - dangerTexture.height * dangerScale + 200.0f; // Reduced from 250 to 200 to move it up slightly
        DrawTextureEx(dangerTexture, {dangerX, dangerY}, 0.0f, dangerScale, WHITE);
    }
    
    drawScarfy();
    drawEntities();
    drawUI();
}


void initGameScore() {
    score = 0;
    level = 1;
    health = 100;
    maxDeaths = 1;
    maxStones = 3;
    maxHearts = 0;
    maxChests = 0;
    treasuresCollectedInLevel = 0;
    heartsCollectedInLevel = 0;
}

void initGame() {
    initGameScore();
    
    loadTextures();
    loadSounds();
    initScarfy();
    initEntities();
}


// ========================= CLEANUP FUNCTIONS =========================
void cleanupGame() {
    UnloadTexture(scarfyTexture);
    UnloadSound(walkSound);
    UnloadSound(landingSound);
    UnloadTexture(coinTexture);
    UnloadTexture(deathTexture);
    UnloadTexture(stoneTexture);
    UnloadTexture(heartTexture);
    UnloadTexture(treasureTexture);
    UnloadTexture(initialBg);
    UnloadTexture(bg2);
    UnloadTexture(bg3);
    UnloadTexture(bg4);
    UnloadTexture(bg5);
    UnloadTexture(bg6);
    UnloadSound(coinSound);
    UnloadSound(hurtSound);
    UnloadTexture(menuBackground);
    UnloadTexture(menuLogo);
    UnloadTexture(playButtonImage);
    UnloadTexture(gameOverBg);
    UnloadTexture(gameOverImage);
    UnloadTexture(playAgainBtn);
    UnloadTexture(scoreCardBtn);
    UnloadTexture(dangerTexture);
    UnloadMusicStream(backgroundMusic);
    UnloadSound(gameOverSound);
}

// ========================= MAIN FUNCTION =========================
int main() {
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "Mario Practice - Functional Version");
    InitAudioDevice();
    SetTargetFPS(60);

    initGame();

    // Start playing background music immediately
    PlayMusicStream(backgroundMusic);

    while (!WindowShouldClose()) {

        // 1. EVENT HANDLING AND UPDATING

        // Update music stream in all states except game over
        if (currentScreen != GAMEOVER) {
            UpdateMusicStream(backgroundMusic);
            // Update music pitch based on level
            if (currentScreen == GAMEPLAY) {
                if (level == 6) {
                    SetMusicPitch(backgroundMusic, slowMusicPitch);
                } else {
                    SetMusicPitch(backgroundMusic, normalMusicPitch);
                }
            }
        }

        // Update game logic ONLY in GAMEPLAY
        if (currentScreen == GAMEPLAY) {
            updateGame();
        } 
        else if (currentScreen == MENU) {
            // Handle menu interactions
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), playButtonBounds)) {
                currentScreen = GAMEPLAY;
                // Initialize game state for first play
                initGameScore();
            }
        }
        else if (currentScreen == GAMEOVER) {
            StopMusicStream(backgroundMusic);
        }

        // 2. Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        if (currentScreen == MENU) {
            drawMenu(menuBackground, menuLogo, playButtonImage);
        }
        else if (currentScreen == GAMEPLAY) {
            drawGame();
        }
        else if (currentScreen == GAMEOVER) {
            drawGameOverMenu(gameOverBg, gameOverImage, playAgainBtn, scoreCardBtn);
        }

        EndDrawing();
    }

    cleanupGame();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}