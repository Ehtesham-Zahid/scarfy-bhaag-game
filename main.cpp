#include<iostream>
#include<raylib.h>
#include<cmath>
using namespace std;

struct Scarfy
{
    Texture2D scarfy;
    Rectangle singleScarfy;
    Vector2 scarfyPosition;
    Vector2 scarfyVelocity;
    Sound walkSound;
    Sound landingSound;

    const float gravity = 1.0f;
    const float scarfySpeed = 10.0f;
    const float jumpStrength = -25.0f;
    
    bool isJumping = false;
    float groundY; // Adjust based on your screen and sprite height
    int scarfyWidth;
    int frameIndex = 0;
    int frameDelay = 5;
    int frameCounter = 0;
    unsigned int numberOfFrames = 6;

    
    Scarfy(){
        scarfy = LoadTexture("./resources/images/scarfy.png");
        walkSound = LoadSound("./resources/sounds/Footstep-A.mp3");
        landingSound = LoadSound("./resources/sounds/Footstep-B.mp3");
        scarfyWidth = scarfy.width / numberOfFrames;
        singleScarfy = {0, 0, (float)scarfyWidth, (float)scarfy.height };
        groundY = (3 * GetScreenHeight()) / 4 - 50;
        scarfyPosition  = {GetScreenWidth() / 2 - scarfyWidth /2, groundY};
        scarfyVelocity = {0.0f, 0.0f};
    }

    void Draw(){
        DrawTextureRec(scarfy, singleScarfy, scarfyPosition , WHITE);
    }

    void CheckGroundCollision() {
        if (scarfyPosition.y >= groundY) {
            scarfyPosition.y = groundY;
            scarfyVelocity.y = 0.0f;
            isJumping = false;
            PlaySound(landingSound);
            // frameIndex = 4; // Optional: Set frame when landed
            // singleScarfy.x = (float)(frameIndex * scarfyWidth);
        }
    }

    void UpdateFrame() {
        frameCounter++;
        if (frameCounter >= frameDelay) {
            frameCounter = 0;
            frameIndex = (frameIndex + 1) % numberOfFrames;
            singleScarfy.x = (float)(frameIndex * scarfyWidth);
            if(frameIndex == 1 || frameIndex == 4) PlaySound(walkSound);
        }
    }

    void Update() {
        // Horizontal movement
        if (IsKeyDown(KEY_RIGHT)) {
            scarfyVelocity.x = scarfySpeed;
            scarfyPosition.x += scarfyVelocity.x;
            if (singleScarfy.width < 0) singleScarfy.width = -singleScarfy.width;
            UpdateFrame();
        } 
        else if (IsKeyDown(KEY_LEFT)) {
            scarfyVelocity.x = scarfySpeed;
            scarfyPosition.x -= scarfyVelocity.x;
            if (singleScarfy.width > 0) singleScarfy.width = -singleScarfy.width;
            UpdateFrame();
        }

        // Jump
        if (IsKeyDown(KEY_UP) && !isJumping) {
            scarfyVelocity.x = 0;
            scarfyVelocity.y = jumpStrength;
            frameIndex = 3; // Jump frame
            singleScarfy.x = (float)(frameIndex * scarfyWidth);
            isJumping = true;
        }

        // Gravity
        scarfyVelocity.y += gravity;
        scarfyPosition.y += scarfyVelocity.y;

        // Ground collision check
        CheckGroundCollision();
    }

};

struct Entity {
    Texture2D texture;
    Vector2 position;
    float speed;
    bool active;
    float damageCooldown;
    float cooldownTime; // 1 second

    

    Entity() : speed(0), active(false) {}

    Entity(Texture2D tex, float x, float y, float s)
        : texture(tex), position({x, y}), speed(s), active(true), damageCooldown(0.0f), cooldownTime(1.0f) {}

    virtual void Update() {
        if (active) {
            position.x += speed;
            if (position.x > GetScreenWidth()) {
                active = false;
            }
        }
        if (damageCooldown > 0.0f) damageCooldown -= GetFrameTime();
    }

    virtual void Draw() const {
        if (active) {
            DrawTexture(texture, position.x, position.y, WHITE);
        }
    }

    Rectangle GetCollisionRect() const {
        return { position.x, position.y, (float)texture.width, (float)texture.height };
    }
};

struct Coin : public Entity {
    Coin() = default;
    Coin(Texture2D tex, float x, float y) : Entity(tex, x, y, 8.0f) {}
};

struct Death : public Entity {
    Death() = default;
    Death(Texture2D tex, float x, float y)
        : Entity(tex, x, y, 8.0f) {}

    
};

struct Stone : public Entity {
    Stone() = default;
    Stone(Texture2D tex, float x, float y)
        : Entity(tex, x, y, 8.0f) {}

    virtual void Update() {
        if (active) {
            position.y += speed;
            if (position.y > GetScreenHeight()) {
                active = false;
            }
        }
    }
};

struct Game {
    Scarfy player;
    Texture2D initialBg;
    Texture2D bg2;
    Texture2D bg3;
    Texture2D bg4;
    Texture2D bg5;
    Texture2D bg6;
    Texture2D coinTexture;
    Texture2D deathTexture;
    Texture2D stoneTexture;

    Sound coinSound;
    Sound hurtSound;


    // static int maxCoins = 6;
    // static int maxDeaths = 6;
    // static int maxStones = 5;

     int maxCoins = 5;
     int maxDeaths = 1;
     int maxStones = 3;

    unsigned int level = 1;
    unsigned int score = 0;
    unsigned int health = 100;

    Coin coins[6];
    Death deaths[6];
    Stone stones[6];

    Game() {
        initialBg = LoadTexture("./resources/images/bg-1.jpg");
        bg2 = LoadTexture("./resources/images/bg-2.jpg");
        bg3 = LoadTexture("./resources/images/bg-3.jpg");
        bg4 = LoadTexture("./resources/images/bg-4.jpg");
        bg5 = LoadTexture("./resources/images/bg-5.jpg");
        bg6 = LoadTexture("./resources/images/bg-6.jpg");
        coinTexture = LoadTexture("./resources/images/coin.png");
        deathTexture = LoadTexture("./resources/images/death.png");
        stoneTexture = LoadTexture("./resources/images/stone.png");

        coinSound = LoadSound("./resources/sounds/coin.mp3");
        hurtSound = LoadSound("./resources/sounds/death.mp3");

        for (int i = 0; i < maxCoins; i++) {
            float screenHeight = GetScreenHeight();
            float y = GetRandomValue(screenHeight / 2 - 50, screenHeight / 2 + 50);
            coins[i] = Coin(coinTexture, GetRandomValue(-500, -100), y); // spawn offscreen
        }

        for (int i = 0; i < maxDeaths; i++) {
            float screenHeight = GetScreenHeight();
            float y = GetRandomValue(screenHeight / 2, screenHeight / 2 + 270);
            deaths[i] = Death(deathTexture, GetRandomValue(-800, -100), y); // spawn offscreen
        }

         for (int i = 0; i < maxStones; i++) {
            float screenWidth = GetScreenWidth();
            float x = GetRandomValue(screenWidth / 4, screenWidth / 4 * 3);
            stones[i] = Stone(stoneTexture, x, GetRandomValue(0, -300)); // spawn offscreen
        }
    }

    void RespawnCoin(int index) {
        float screenHeight = GetScreenHeight();
        float y = GetRandomValue(screenHeight / 2 - 50, screenHeight / 2 + 50);
        coins[index] = Coin(coinTexture, GetRandomValue(-500, -100), y); // spawn offscreen
    }

    void RespawnDeath(int index) {
        float screenHeight = GetScreenHeight();
        float y = GetRandomValue(screenHeight / 2 , screenHeight / 2 + 270);
        deaths[index] = Death(deathTexture, GetRandomValue(-800, -100), y); // spawn offscreen
    }

    void RespawnStone(int index) {
        float screenWidth = GetScreenWidth();
        float x = GetRandomValue(screenWidth / 4, screenWidth / 4 * 3);
        stones[index] = Stone(stoneTexture, x, GetRandomValue(0, -300)); // spawn offscreen
    }


    void Update() {
        player.Update();

        for (int i = 0; i < maxCoins; i++) {
            coins[i].Update();

            Rectangle coinRect = coins[i].GetCollisionRect();
            Rectangle scarfyRect = {
                player.scarfyPosition.x,
                player.scarfyPosition.y,
                (float)player.scarfyWidth,
                (float)player.scarfy.height
            };

            if (coins[i].active && CheckCollisionRecs(coinRect, scarfyRect)) {
                PlaySound(coinSound);
                coins[i].active = false;
                score++;
            }

            if (!coins[i].active || coins[i].position.x > GetScreenWidth()) {
                RespawnCoin(i);
            }
        }

        for (int i = 0; i < maxDeaths; i++) {
            deaths[i].Update();

            Rectangle deathRect = deaths[i].GetCollisionRect();
            Rectangle scarfyRect = {
                player.scarfyPosition.x,
                player.scarfyPosition.y,
                (float)player.scarfyWidth,
                (float)player.scarfy.height
            };

            if (deaths[i].active && CheckCollisionRecs(deathRect, scarfyRect)) {
                if (deaths[i].damageCooldown <= 0.0f) {
                    PlaySound(hurtSound);
                    health -= 7;
                    deaths[i].damageCooldown = deaths[i].cooldownTime;
                }
            }

            if (!deaths[i].active || deaths[i].position.x > GetScreenWidth()) {
                RespawnDeath(i);
            }
        }

         for (int i = 0; i < maxStones; i++) {
            stones[i].Update();

            Rectangle stoneRect = stones[i].GetCollisionRect();
            Rectangle scarfyRect = {
                player.scarfyPosition.x,
                player.scarfyPosition.y,
                (float)player.scarfyWidth,
                (float)player.scarfy.height
            };

           if (stones[i].active && CheckCollisionRecs(stoneRect, scarfyRect)) {
                if (stones[i].damageCooldown <= 0.0f) {
                    PlaySound(hurtSound);
                    health -= 3;
                    stones[i].damageCooldown = stones[i].cooldownTime;
                }
            }

            if (!stones[i].active || stones[i].position.x > GetScreenWidth()) {
                RespawnStone(i);
            }
        }
        if(score > 20 && score <= 40){
            level = 2;
            maxDeaths = 2;
            maxStones = 3;
        } else if(score > 40 && score <= 60){
            level = 3;
            maxDeaths = 3;
            maxStones = 4;
        } else if(score > 60 && score <= 75){
            level = 4;
            maxDeaths = 3;
            maxStones = 4;
        } else if (score > 75 && score <= 90){
            level = 5;
            maxDeaths = 4;
            maxStones = 5;
        }  else if (score > 90){
            level = 6;
            maxDeaths = 5;
            maxStones = 5;
        }
    }

    void DrawBackground(Texture2D bg){
        float bgScaleX = (float)GetScreenWidth() / bg.width;
        float bgScaleY = (float)GetScreenHeight() / bg.height;
        float scale = fmax(bgScaleX, bgScaleY);

        float offsetX = (GetScreenWidth() - bg.width * scale) / 2.0f;
        float offsetY = (GetScreenHeight() - bg.height * scale) / 2.0f;

        DrawTextureEx(bg, { offsetX, offsetY }, 0.0f, scale, WHITE);
    }
    void Draw() {
       
        if(level == 1){
            DrawBackground(initialBg);
        } else if(level == 2){
            DrawBackground(bg2);
        }else if(level == 3){
            DrawBackground(bg3);
        }else if(level == 4){
            DrawBackground(bg4);
        }else if(level == 5){
            DrawBackground(bg5);
        }else if(level == 6){
            DrawBackground(bg6);
        }
        player.Draw();

        for (int i = 0; i < maxCoins; i++) {
            coins[i].Draw();
        }

        for (int i = 0; i < maxDeaths; i++) {
            deaths[i].Draw();
        }

        for (int i = 0; i < maxStones; i++) {
            stones[i].Draw();
        }
    }

    void Cleanup() {
        UnloadTexture(player.scarfy);
        UnloadSound(player.walkSound);
        UnloadSound(player.landingSound);
        UnloadTexture(coinTexture);
        UnloadTexture(initialBg);
    }
};

int main()
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "Mario Practice");
    InitAudioDevice();
    SetTargetFPS(60);

    Game game;

    while(WindowShouldClose() == false){
        // 1. Event Handling

        // 2. Updting
        game.Update();

        // 3. Drawing

        BeginDrawing();
        ClearBackground(BLACK);
        game.Draw();
        if(game.level == 6){
            DrawText("Final Level", GetScreenWidth() / 2, 50 , 60, BLACK);

        } else{
            DrawText(TextFormat("Level: %i", game.level), GetScreenWidth() / 2, 50 , 60, BLACK);
        }
        DrawText(TextFormat("Score: %i", game.score), 40, 20 , 60, WHITE);
        DrawText(TextFormat("Health: %i", game.health), 40, 70 , 60, WHITE);
        EndDrawing();
    }


    CloseAudioDevice();
    CloseWindow();
    return 0;
}