/**
 * Author: Xuanyi Li
 * Assignment: Pong Clone
 * Date due: 10/16/2026
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 */

#include "raylib.h"
#include <math.h>
#include <cstdio>

enum AppStatus { TERMINATED, RUNNING };
constexpr int SCREEN_WIDTH = 1000, SCREEN_HEIGHT = 600, FPS = 60;
constexpr int MAX_BALLS = 3, WINNING_SCORE = 5;
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
constexpr float PLAYER_SPEED = 360.0f, AUTO_SPEED = 280.0f;
constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
constexpr Vector2 PLAYER_SIZE = { 48.0f, 140.0f };
constexpr Vector2 BALL_SIZE = { 28.0f, 28.0f };
constexpr char PLAYER_FP[] = "assets/game/paddle.png";
constexpr char BALL_FP[] = "assets/game/ball.png";

AppStatus gAppStatus = RUNNING;
Vector2 gLeftPosition = { 60.0f, ORIGIN.y };
Vector2 gRightPosition = { SCREEN_WIDTH - 60.0f, ORIGIN.y };
Vector2 *gBallPositions = nullptr;
Vector2 *gBallVelocities = nullptr;
float gLeftMovement = 0.0f, gRightMovement = 0.0f;
float gAutoDirection = 1.0f;
float gPreviousTicks = 0.0f, gTimeAccumulator = 0.0f;
int gBallCount = 1, gLeftScore = 0, gRightScore = 0;
bool gSinglePlayer = false, gGameOver = false;
Texture2D gPlayerTexture, gBallTexture;
Texture2D gBackgroundTexture, gLeftWinTexture, gRightWinTexture;

void initialise();
void processInput();
void update();
void render();
void shutdown();
void resetBall(int index);
void resetGame();
void movePlayers(float deltaTime);
void updateBall(int index, float deltaTime);
void changeBallCount(int count);
bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB);
void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale);

bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB)
{
    float xDistance = fabs(positionA->x - positionB->x)
                      - (scaleA->x + scaleB->x) / 2.0f;
    float yDistance = fabs(positionA->y - positionB->y)
                      - (scaleA->y + scaleB->y) / 2.0f;
    return xDistance < 0.0f && yDistance < 0.0f;
}

void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale)
{
    Rectangle textureArea = { 0.0f, 0.0f,
        static_cast<float>(texture->width), static_cast<float>(texture->height) };
    Rectangle destinationArea = { position->x, position->y, scale->x, scale->y };
    Vector2 originOffset = { scale->x / 2.0f, scale->y / 2.0f };
    DrawTexturePro(*texture, textureArea, destinationArea, originOffset, 0.0f, WHITE);
}

void resetBall(int index)
{
    gBallPositions[index] = { ORIGIN.x, 180.0f + index * 120.0f };
    gBallVelocities[index] = { 320.0f, 180.0f };
    if (index == 1) gBallVelocities[index] = { -350.0f, -200.0f };
    if (index == 2) gBallVelocities[index] = { 380.0f, -220.0f };
}

void resetGame()
{
    gLeftPosition = { 60.0f, ORIGIN.y };
    gRightPosition = { SCREEN_WIDTH - 60.0f, ORIGIN.y };
    gLeftScore = 0;
    gRightScore = 0;
    gGameOver = false;
    gAutoDirection = 1.0f;
    gTimeAccumulator = 0.0f;
    gPreviousTicks = static_cast<float>(GetTime());
    for (int i = 0; i < MAX_BALLS; i++) resetBall(i);
    printf("New game. Left: 0 | Right: 0. First to 5 wins.\n");
}

void changeBallCount(int count)
{
    for (int i = gBallCount; i < count; i++) resetBall(i);
    gBallCount = count;
    printf("Active balls: %d\n", gBallCount);
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Pong | W/S vs Up/Down | T: mode | 1-3: balls | R: restart");
    gBallPositions = new Vector2[MAX_BALLS];
    gBallVelocities = new Vector2[MAX_BALLS];
    gPlayerTexture = LoadTexture(PLAYER_FP);
    gBallTexture = LoadTexture(BALL_FP);
    gBackgroundTexture = LoadTexture("assets/game/space.png");
    gLeftWinTexture = LoadTexture("assets/game/player1_wins.png");
    gRightWinTexture = LoadTexture("assets/game/player2_wins.png");
    SetTargetFPS(FPS);
    resetGame();
    printf("W/S: left player. Up/Down: right player.\n");
    printf("T: single/two player. 1/2/3: ball count. R: restart. Q: quit.\n");
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
    if (IsKeyPressed(KEY_R)) resetGame();

    gLeftMovement = 0.0f;
    gRightMovement = 0.0f;
    if (gGameOver) return;

    if (IsKeyPressed(KEY_T))
    {
        gSinglePlayer = !gSinglePlayer;
        if (gSinglePlayer) printf("Single player: right paddle moves automatically.\n");
        else printf("Two players: right paddle uses Up/Down.\n");
    }
    if (IsKeyPressed(KEY_ONE)) changeBallCount(1);
    if (IsKeyPressed(KEY_TWO)) changeBallCount(2);
    if (IsKeyPressed(KEY_THREE)) changeBallCount(3);

    if      (IsKeyDown(KEY_W)) gLeftMovement = -1.0f;
    else if (IsKeyDown(KEY_S)) gLeftMovement = 1.0f;
    if (!gSinglePlayer)
    {
        if      (IsKeyDown(KEY_UP)) gRightMovement = -1.0f;
        else if (IsKeyDown(KEY_DOWN)) gRightMovement = 1.0f;
    }
}

void movePlayers(float deltaTime)
{
    gLeftPosition.y += PLAYER_SPEED * gLeftMovement * deltaTime;
    if (gSinglePlayer) gRightPosition.y += AUTO_SPEED * gAutoDirection * deltaTime;
    else gRightPosition.y += PLAYER_SPEED * gRightMovement * deltaTime;

    float top = PLAYER_SIZE.y / 2.0f;
    float bottom = SCREEN_HEIGHT - PLAYER_SIZE.y / 2.0f;
    if (gLeftPosition.y < top) gLeftPosition.y = top;
    if (gLeftPosition.y > bottom) gLeftPosition.y = bottom;
    if (gRightPosition.y <= top)
    {
        gRightPosition.y = top;
        gAutoDirection = 1.0f;
    }
    if (gRightPosition.y >= bottom)
    {
        gRightPosition.y = bottom;
        gAutoDirection = -1.0f;
    }
}

void updateBall(int index, float deltaTime)
{
    gBallPositions[index].x += gBallVelocities[index].x * deltaTime;
    gBallPositions[index].y += gBallVelocities[index].y * deltaTime;

    float halfBall = BALL_SIZE.y / 2.0f;
    if (gBallPositions[index].y <= halfBall)
    {
        gBallPositions[index].y = halfBall;
        if (gBallVelocities[index].y < 0.0f) gBallVelocities[index].y *= -1.0f;
    }
    if (gBallPositions[index].y >= SCREEN_HEIGHT - halfBall)
    {
        gBallPositions[index].y = SCREEN_HEIGHT - halfBall;
        if (gBallVelocities[index].y > 0.0f) gBallVelocities[index].y *= -1.0f;
    }

    if (gBallVelocities[index].x < 0.0f &&
        isColliding(&gBallPositions[index], &BALL_SIZE, &gLeftPosition, &PLAYER_SIZE))
    {
        gBallPositions[index].x = gLeftPosition.x + (PLAYER_SIZE.x + BALL_SIZE.x) / 2.0f;
        gBallVelocities[index].x *= -1.0f;
    }
    if (gBallVelocities[index].x > 0.0f &&
        isColliding(&gBallPositions[index], &BALL_SIZE, &gRightPosition, &PLAYER_SIZE))
    {
        gBallPositions[index].x = gRightPosition.x - (PLAYER_SIZE.x + BALL_SIZE.x) / 2.0f;
        gBallVelocities[index].x *= -1.0f;
    }

    bool scored = false;
    if (gBallPositions[index].x < -BALL_SIZE.x / 2.0f)
    {
        gRightScore++;
        scored = true;
    }
    else if (gBallPositions[index].x > SCREEN_WIDTH + BALL_SIZE.x / 2.0f)
    {
        gLeftScore++;
        scored = true;
    }
    if (scored)
    {
        printf("Left: %d | Right: %d\n", gLeftScore, gRightScore);
        if (gLeftScore >= WINNING_SCORE || gRightScore >= WINNING_SCORE)
        {
            gGameOver = true;
            if (gLeftScore >= WINNING_SCORE) printf("LEFT PLAYER WINS! R to restart.\n");
            else printf("RIGHT PLAYER WINS! R to restart.\n");
        }
        else resetBall(index);
    }
}

void update()
{
    float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;
    if (gGameOver) return;

    deltaTime += gTimeAccumulator;
    while (deltaTime >= FIXED_TIMESTEP && !gGameOver)
    {
        movePlayers(FIXED_TIMESTEP);
        for (int i = 0; i < gBallCount && !gGameOver; i++)
            updateBall(i, FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }
    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    Vector2 backgroundSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
    renderObject(&gBackgroundTexture, &ORIGIN, &backgroundSize);
    renderObject(&gPlayerTexture, &gLeftPosition, &PLAYER_SIZE);
    renderObject(&gPlayerTexture, &gRightPosition, &PLAYER_SIZE);
    for (int i = 0; i < gBallCount; i++)
        renderObject(&gBallTexture, &gBallPositions[i], &BALL_SIZE);
    if (gGameOver)
    {
        Vector2 messageSize = { 700.0f, 300.0f };
        if (gLeftScore >= WINNING_SCORE)
            renderObject(&gLeftWinTexture, &ORIGIN, &messageSize);
        else
            renderObject(&gRightWinTexture, &ORIGIN, &messageSize);
    }
    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gPlayerTexture);
    UnloadTexture(gBallTexture);
    UnloadTexture(gBackgroundTexture);
    UnloadTexture(gLeftWinTexture);
    UnloadTexture(gRightWinTexture);
    delete[] gBallPositions;
    delete[] gBallVelocities;
    CloseWindow();
}

int main(void)
{
    initialise();
    while (gAppStatus == RUNNING)
    {
        processInput();
        if (gAppStatus == RUNNING)
        {
            update();
            render();
        }
    }
    shutdown();
    return 0;
}
