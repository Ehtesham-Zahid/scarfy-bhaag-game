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

struct Coin {
    Texture2D texture;
    Vector2 position;
    float coinSpeed = 8.0f;
    bool active;


    Coin(){}
    // Constructor
    Coin(Texture2D tex, float x, float y) {
        texture = tex;
        position = { x, y };
        active = true;
    }

    void Update() {
        if (active) {
            position.x += coinSpeed;
            if (position.x > GetScreenWidth()) {
                active = false;
            }
        }
    }

    void Draw() {
        if (active) {
            DrawTexture(texture, position.x, position.y, WHITE);
        }
    }

    Rectangle GetCollisionRect() {
        return { position.x, position.y, (float)texture.width, (float)texture.height };
    }
};



struct Game {
    Scarfy player;
    Texture2D initialBg;
    Texture2D coinTexture;
    static const int maxCoins = 6;
    Coin coins[maxCoins];

    Game() {
        initialBg = LoadTexture("./resources/images/bg-1.jpg");
        coinTexture = LoadTexture("./resources/images/coin.png");

        for (int i = 0; i < maxCoins; i++) {
            float screenHeight = GetScreenHeight();
            float y = GetRandomValue(screenHeight / 2 - 50, screenHeight / 2 + 50);
            coins[i] = Coin(coinTexture, GetRandomValue(-400, -100), y); // spawn offscreen
        }
    }

    void RespawnCoin(int index) {
        float screenHeight = GetScreenHeight();
        float y = GetRandomValue(screenHeight / 2 - 50, screenHeight / 2 + 50);
        coins[index] = Coin(coinTexture, GetRandomValue(-400, -100), y); // spawn offscreen
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
                coins[i].active = false;
            }

            if (!coins[i].active || coins[i].position.x > GetScreenWidth()) {
                RespawnCoin(i);
            }
        }
    }

    void Draw() {
        float bgScaleX = (float)GetScreenWidth() / initialBg.width;
        float bgScaleY = (float)GetScreenHeight() / initialBg.height;
        float scale = fmax(bgScaleX, bgScaleY);

        float offsetX = (GetScreenWidth() - initialBg.width * scale) / 2.0f;
        float offsetY = (GetScreenHeight() - initialBg.height * scale) / 2.0f;

        DrawTextureEx(initialBg, { offsetX, offsetY }, 0.0f, scale, WHITE);
        player.Draw();

        for (int i = 0; i < maxCoins; i++) {
            coins[i].Draw();
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
        EndDrawing();
    }


    CloseAudioDevice();
    CloseWindow();
    return 0;
}