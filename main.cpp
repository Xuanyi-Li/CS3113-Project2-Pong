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
#include <cmath>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const float PADDLE_SPEED = 320.0f;
const float BALL_SPEED = 260.0f;
const int MAX_BALLS = 3;
const int WIN_SCORE = 5;

struct Ball {
    Vector2 position;
    Vector2 velocity;
};

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong Variations");
    SetTargetFPS(60);

    Texture2D paddleTexture = LoadTexture("paddle.png");
    Texture2D ballTexture = LoadTexture("ball.png");

    Rectangle left = { 35, 175, 18, 100 };
    Rectangle right = { 747, 175, 18, 100 };
    Ball balls[MAX_BALLS];
    int ballCount = 1;
    int leftScore = 0;
    int rightScore = 0;
    bool singlePlayer = false;
    bool gameOver = false;

    for (int i = 0; i < MAX_BALLS; i++) {
        balls[i].position = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f + i * 35.0f };
        balls[i].velocity = { (i % 2 == 0 ? BALL_SPEED : -BALL_SPEED), (i % 2 == 0 ? 150.0f : -150.0f) };
    }

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (!gameOver) {
            if (IsKeyPressed(KEY_T)) singlePlayer = !singlePlayer;
            if (IsKeyPressed(KEY_ONE)) ballCount = 1;
            if (IsKeyPressed(KEY_TWO)) ballCount = 2;
            if (IsKeyPressed(KEY_THREE)) ballCount = 3;

            if (IsKeyDown(KEY_W)) left.y -= PADDLE_SPEED * dt;
            if (IsKeyDown(KEY_S)) left.y += PADDLE_SPEED * dt;

            if (singlePlayer) {
                float target = balls[0].position.y - right.height / 2;
                if (right.y < target - 4) right.y += PADDLE_SPEED * dt;
                if (right.y > target + 4) right.y -= PADDLE_SPEED * dt;
            } else {
                if (IsKeyDown(KEY_UP)) right.y -= PADDLE_SPEED * dt;
                if (IsKeyDown(KEY_DOWN)) right.y += PADDLE_SPEED * dt;
            }

            if (left.y < 0) left.y = 0;
            if (right.y < 0) right.y = 0;
            if (left.y + left.height > SCREEN_HEIGHT) left.y = SCREEN_HEIGHT - left.height;
            if (right.y + right.height > SCREEN_HEIGHT) right.y = SCREEN_HEIGHT - right.height;

            for (int i = 0; i < ballCount; i++) {
                Ball &ball = balls[i];
                ball.position.x += ball.velocity.x * dt;
                ball.position.y += ball.velocity.y * dt;

                Rectangle ballBox = {ball.position.x - 10, ball.position.y - 10, 20, 20};

                if (ball.position.y <= 10 && ball.velocity.y < 0) {
                    ball.position.y = 10;
                    ball.velocity.y *= -1;
                }
                if (ball.position.y >= SCREEN_HEIGHT - 10 && ball.velocity.y > 0) {
                    ball.position.y = SCREEN_HEIGHT - 10;
                    ball.velocity.y *= -1;
                }

                if (CheckCollisionRecs(ballBox, left) && ball.velocity.x < 0) {
                    ball.position.x = left.x + left.width + 10;
                    ball.velocity.x *= -1;
                }
                if (CheckCollisionRecs(ballBox, right) && ball.velocity.x > 0) {
                    ball.position.x = right.x - 10;
                    ball.velocity.x *= -1;
                }

                if (ball.position.x < -10 || ball.position.x > SCREEN_WIDTH + 10) {
                    if (ball.position.x < 0) rightScore++;
                    else leftScore++;
                    ball.position = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f + i * 35.0f };
                    ball.velocity = { (i % 2 == 0 ? BALL_SPEED : -BALL_SPEED), (i % 2 == 0 ? 150.0f : -150.0f) };
                    if (leftScore >= WIN_SCORE || rightScore >= WIN_SCORE) {
                        gameOver = true;
                        break;
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground((Color){ 20, 23, 42, 255 });
        for (int y = 0; y < SCREEN_HEIGHT; y += 28) DrawRectangle(SCREEN_WIDTH / 2 - 2, y, 4, 14, (Color){ 90, 95, 120, 255 });
        DrawTexturePro(paddleTexture, { 0, 0, (float)paddleTexture.width, (float)paddleTexture.height }, left, { 0, 0 }, 0, WHITE);
        DrawTexturePro(paddleTexture, { 0, 0, (float)paddleTexture.width, (float)paddleTexture.height }, right, { 0, 0 }, 0, WHITE);
        for (int i = 0; i < ballCount; i++) {
            Rectangle dest = { balls[i].position.x - 10, balls[i].position.y - 10, 20, 20 };
            DrawTexturePro(ballTexture, { 0, 0, (float)ballTexture.width, (float)ballTexture.height }, dest, { 0, 0 }, 0, WHITE);
        }
        DrawText(TextFormat("%d", leftScore), 290, 20, 40, RAYWHITE);
        DrawText(TextFormat("%d", rightScore), 485, 20, 40, RAYWHITE);
        DrawText(singlePlayer ? "1 PLAYER (T TO SWITCH)" : "2 PLAYERS (T TO SWITCH)", 12, 12, 16, LIGHTGRAY);
        DrawText("W/S    UP/DOWN     1/2/3 BALLS", 12, SCREEN_HEIGHT - 22, 14, LIGHTGRAY);
        if (gameOver) {
            DrawRectangle(160, 160, 480, 130, (Color){ 20, 23, 42, 240 });
            DrawText(leftScore >= WIN_SCORE ? "PLAYER 1 WINS!" : "PLAYER 2 WINS!", 235, 208, 32, GOLD);
        }
        EndDrawing();
    }
    UnloadTexture(paddleTexture);
    UnloadTexture(ballTexture);
    CloseWindow();
    return 0;
}
