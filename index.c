#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Define constants for virtual resolution and maximum limits
#define VIRTUAL_WIDTH 1000
#define VIRTUAL_HEIGHT 600
#define MAX_LASERS 100
#define MAX_ENEMIES 10
#define MAX_PARTICLES 100

// Define structures for game objects
typedef struct {
    Vector2 position;   // Position of the spaceship
    float rotation;     // Rotation angle in degrees
    float speed;        // Movement speed
} Spaceship;

typedef struct {
    Vector2 position;   // Position of the laser
    float rotation;     // Rotation angle in degrees
    float speed;        // Movement speed
    bool active;        // Whether the laser is active
} Laser;

typedef struct {
    Vector2 position;   // Position of the enemy
    float speed;        // Movement speed
    bool active;        // Whether the enemy is active
} Enemy;

typedef struct {
    Vector2 position;   // Position of the particle
    Vector2 velocity;   // Movement direction and speed
    Color color;        // Color of the particle
    float alpha;        // Transparency (0.0 to 1.0)
    float life;         // Remaining lifetime
} Particle;

int main(void) {
    // Initialize Raylib window
    InitWindow(VIRTUAL_WIDTH, VIRTUAL_HEIGHT, "Fun Internet");
    SetTargetFPS(60);

    // Create render texture for virtual resolution
    RenderTexture2D target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

    // Initialize player spaceship at the center of the screen
    Spaceship player = { {VIRTUAL_WIDTH / 2, VIRTUAL_HEIGHT / 2}, 0.0f, 0.0f };

    // Initialize arrays for lasers, enemies, and particles
    Laser lasers[MAX_LASERS] = {0};
    Enemy enemies[MAX_ENEMIES] = {0};
    Particle particles[MAX_PARTICLES] = {0};

    // Game state variables
    int score = 0;              // Player's score
    bool playerExploded = false; // Whether the player has exploded (game over)

    // Define the "Try Again" button rectangle
    Rectangle tryAgainButton = { (VIRTUAL_WIDTH - 200) / 2, 300, 200, 50 };

    // Seed random number generator for enemy spawning and particle effects
    srand(time(NULL));

    // Main game loop
    while (!WindowShouldClose()) {
        // Calculate scaling and positioning for resized window
        float scale = fminf((float)GetScreenWidth() / VIRTUAL_WIDTH, (float)GetScreenHeight() / VIRTUAL_HEIGHT);
        Rectangle destRec = {
            (GetScreenWidth() - VIRTUAL_WIDTH * scale) / 2,
            (GetScreenHeight() - VIRTUAL_HEIGHT * scale) / 2,
            VIRTUAL_WIDTH * scale,
            VIRTUAL_HEIGHT * scale
        };

        // Adjust mouse position to virtual coordinates
        Vector2 mousePos = GetMousePosition();
        mousePos.x = (mousePos.x - destRec.x) / scale;
        mousePos.y = (mousePos.y - destRec.y) / scale;

        // **Update Game Logic**
        if (!playerExploded) {
            // Player movement and rotation controls
            if (IsKeyDown(KEY_RIGHT)) player.rotation += 5.0f;
            if (IsKeyDown(KEY_LEFT)) player.rotation -= 5.0f;
            if (IsKeyDown(KEY_UP)) player.speed += 0.1f;
            if (IsKeyDown(KEY_DOWN)) player.speed -= 0.1f;
            player.speed = (player.speed < 0.0f) ? 0.0f : (player.speed > 10.0f) ? 10.0f : player.speed;
            float rad = player.rotation * DEG2RAD;
            player.position.x += player.speed * cosf(rad);
            player.position.y += player.speed * sinf(rad);
            // Wrap around screen edges
            player.position.x = (player.position.x > VIRTUAL_WIDTH) ? 0 : (player.position.x < 0) ? VIRTUAL_WIDTH : player.position.x;
            player.position.y = (player.position.y > VIRTUAL_HEIGHT) ? 0 : (player.position.y < 0) ? VIRTUAL_HEIGHT : player.position.y;

            // Shoot laser when spacebar is pressed
            if (IsKeyPressed(KEY_SPACE)) {
                for (int i = 0; i < MAX_LASERS; i++) {
                    if (!lasers[i].active) {
                        lasers[i].position = (Vector2){player.position.x + 20 * cosf(rad), player.position.y + 20 * sinf(rad)};
                        lasers[i].rotation = player.rotation;
                        lasers[i].speed = 10.0f;
                        lasers[i].active = true;
                        break;
                    }
                }
            }

            // Update laser positions
            for (int i = 0; i < MAX_LASERS; i++) {
                if (lasers[i].active) {
                    float laserRad = lasers[i].rotation * DEG2RAD;
                    lasers[i].position.x += lasers[i].speed * cosf(laserRad);
                    lasers[i].position.y += lasers[i].speed * sinf(laserRad);
                    if (lasers[i].position.x < 0 || lasers[i].position.x > VIRTUAL_WIDTH ||
                        lasers[i].position.y < 0 || lasers[i].position.y > VIRTUAL_HEIGHT) {
                        lasers[i].active = false;
                    }
                }
            }

            // Spawn enemies periodically
            static float enemySpawnTimer = 0.0f;
            enemySpawnTimer += GetFrameTime();
            if (enemySpawnTimer > 2.0f) {
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (!enemies[i].active) {
                        enemies[i].position.x = rand() % VIRTUAL_WIDTH;
                        enemies[i].position.y = rand() % VIRTUAL_HEIGHT;
                        enemies[i].speed = 2.0f;
                        enemies[i].active = true;
                        break;
                    }
                }
                enemySpawnTimer = 0.0f;
            }

            // Update enemy movement towards player
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    Vector2 direction = {player.position.x - enemies[i].position.x, player.position.y - enemies[i].position.y};
                    float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
                    if (length > 0) {
                        direction.x /= length;
                        direction.y /= length;
                    }
                    enemies[i].position.x += direction.x * enemies[i].speed;
                    enemies[i].position.y += direction.y * enemies[i].speed;
                }
            }

            // Check collisions between lasers and enemies
            for (int i = 0; i < MAX_LASERS; i++) {
                if (lasers[i].active) {
                    for (int j = 0; j < MAX_ENEMIES; j++) {
                        if (enemies[j].active && CheckCollisionPointCircle(lasers[i].position, enemies[j].position, 10)) {
                            lasers[i].active = false;
                            enemies[j].active = false;
                            score++;
                            // Create explosion effect for destroyed enemy
                            for (int k = 0; k < 10; k++) {
                                int p = rand() % MAX_PARTICLES;
                                if (particles[p].life <= 0) {
                                    particles[p].position = enemies[j].position;
                                    float angle = rand() % 360 * DEG2RAD;
                                    float speed = (rand() % 5) + 1;
                                    particles[p].velocity = (Vector2){speed * cosf(angle), speed * sinf(angle)};
                                    particles[p].color = YELLOW;
                                    particles[p].alpha = 1.0f;
                                    particles[p].life = 1.0f;
                                }
                            }
                        }
                    }
                }
            }

            // Check collision between player and enemies (game over condition)
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active && CheckCollisionPointCircle(player.position, enemies[i].position, 10)) {
                    playerExploded = true;
                    // Create explosion effect for player
                    for (int k = 0; k < 20; k++) {
                        int p = rand() % MAX_PARTICLES;
                        if (particles[p].life <= 0) {
                            particles[p].position = player.position;
                            float angle = rand() % 360 * DEG2RAD;
                            float speed = (rand() % 5) + 1;
                            particles[p].velocity = (Vector2){speed * cosf(angle), speed * sinf(angle)};
                            particles[p].color = RED;
                            particles[p].alpha = 1.0f;
                            particles[p].life = 1.0f;
                        }
                    }
                }
            }
        }

        // **Handle "Try Again" button click and reset game**
        if (playerExploded) {
            bool isHovered = CheckCollisionPointRec(mousePos, tryAgainButton);
            if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Reset player to initial state
                player.position = (Vector2){VIRTUAL_WIDTH / 2, VIRTUAL_HEIGHT / 2};
                player.rotation = 0.0f;
                player.speed = 0.0f;
                // Deactivate all lasers and enemies
                for (int i = 0; i < MAX_LASERS; i++) {
                    lasers[i].active = false;
                }
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    enemies[i].active = false;
                }
                // Reset score and game state
                score = 0;
                playerExploded = false;
            }
        }

        // Update particles for fireworks and explosions
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life > 0) {
                particles[i].position.x += particles[i].velocity.x;
                particles[i].position.y += particles[i].velocity.y;
                particles[i].alpha -= 0.01f;
                particles[i].life -= 0.01f;
                if (particles[i].alpha < 0) particles[i].alpha = 0;
            }
        }

        // Generate background fireworks
        static float fireworkTimer = 0.0f;
        fireworkTimer += GetFrameTime();
        if (fireworkTimer > 0.1f) {
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life <= 0) {
                    particles[i].position = (Vector2){rand() % VIRTUAL_WIDTH, rand() % VIRTUAL_HEIGHT};
                    float angle = rand() % 360 * DEG2RAD;
                    float speed = (rand() % 5) + 1;
                    particles[i].velocity = (Vector2){speed * cosf(angle), speed * sinf(angle)};
                    particles[i].color = WHITE;
                    particles[i].alpha = 1.0f;
                    particles[i].life = 1.0f;
                    break;
                }
            }
            fireworkTimer = 0.0f;
        }

        // **Draw to Render Texture**
        BeginTextureMode(target);
        ClearBackground(BLACK);

        // Draw background fireworks and explosion particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life > 0) {
                DrawPixelV(particles[i].position, Fade(particles[i].color, particles[i].alpha));
            }
        }

        // Draw player spaceship if not exploded
        if (!playerExploded) {
            float rad = player.rotation * DEG2RAD;
            Vector2 front = {player.position.x + 20 * cosf(rad), player.position.y + 20 * sinf(rad)};
            Vector2 backLeft = {player.position.x - 10 * cosf(rad) + 10 * sinf(rad), player.position.y - 10 * sinf(rad) - 10 * cosf(rad)};
            Vector2 backRight = {player.position.x - 10 * cosf(rad) - 10 * sinf(rad), player.position.y - 10 * sinf(rad) + 10 * cosf(rad)};
            DrawTriangle(front, backLeft, backRight, WHITE);
        }

        // Draw active lasers
        for (int i = 0; i < MAX_LASERS; i++) {
            if (lasers[i].active) {
                float laserRad = lasers[i].rotation * DEG2RAD;
                Vector2 end = {lasers[i].position.x + 10 * cosf(laserRad), lasers[i].position.y + 10 * sinf(laserRad)};
                DrawLineV(lasers[i].position, end, YELLOW);
            }
        }

        // Draw active enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                DrawCircleV(enemies[i].position, 10, RED);
            }
        }

        // Draw animated title text
        static float timer = 0.0f;
        timer += GetFrameTime();
        Color colors[] = {BLUE, RED, GREEN, WHITE, MAGENTA};
        int colorIndex = (int)(timer * 2) % 5;
        int textWidth = MeasureText("RAGE AGAINST THE DYING INTERNET", 20);
        DrawText("RAGE AGAINST THE DYING INTERNET", (VIRTUAL_WIDTH - textWidth) / 2, 10, 20, colors[colorIndex]);

        // Draw additional UI text
        textWidth = MeasureText("Make your own website, fill with ur SOUL & fuck the SLOP!", 10);
        DrawText("Make your own website, fill with ur SOUL & fuck the SLOP!", (VIRTUAL_WIDTH - textWidth) / 2, 40, 10, WHITE);

        textWidth = MeasureText("ARROW and SPACE to control your Spaceship", 10);
        DrawText("ARROW and SPACE to control your Spaceship", (VIRTUAL_WIDTH - textWidth) / 2, 60, 10, Fade(WHITE, 0.5f));

        // Draw social media button
        Rectangle buttonRect = {(VIRTUAL_WIDTH - 100) / 2, 80, 100, 20};
        bool isHovered = CheckCollisionPointRec(mousePos, buttonRect);
        Color buttonColor = isHovered ? YELLOW : WHITE;
        DrawRectangleRec(buttonRect, buttonColor);
        int buttonTextWidth = MeasureText("X kirbara2000", 10);
        int textX = buttonRect.x + (buttonRect.width - buttonTextWidth) / 2;
        int textY = buttonRect.y + (buttonRect.height - 10) / 2;
        DrawText("X kirbara2000", textX, textY, 10, BLACK);
        if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            OpenURL("https://x.com/kirbara2000");
        }

        // Draw score or game over message
        if (!playerExploded) {
            char scoreText[32];
            sprintf(scoreText, "Score: %d", score);
            textWidth = MeasureText(scoreText, 10);
            DrawText(scoreText, (VIRTUAL_WIDTH - textWidth) / 2, VIRTUAL_HEIGHT - 30, 10, YELLOW);
        } else {
            char gameOverText[64];
            sprintf(gameOverText, "IT'S SO OVER. TOTAL: %d", score);
            textWidth = MeasureText(gameOverText, 10);
            DrawText(gameOverText, (VIRTUAL_WIDTH - textWidth) / 2, VIRTUAL_HEIGHT - 30, 10, YELLOW);
        }

        // **Draw "Try Again" button when game is over**
        if (playerExploded) {
            bool isHovered = CheckCollisionPointRec(mousePos, tryAgainButton);
            Color buttonColor = isHovered ? YELLOW : WHITE;
            DrawRectangleRec(tryAgainButton, buttonColor);
            textWidth = MeasureText("Nah, I'd Win", 20);
            textX = tryAgainButton.x + (tryAgainButton.width - textWidth) / 2;
            textY = tryAgainButton.y + (tryAgainButton.height - 20) / 2;
            DrawText("Nah, I'd Win", textX, textY, 20, BLACK);
        }

        // Draw bottom text
        textWidth = MeasureText("This site is written in C, Raylib, Wasm, & Grok. Techzz is crazyy. U can do THINGS!", 10);
        DrawText("This site is written in C, Raylib, Wasm, & Grok. Techzz is crazyy. U can do THINGS!", (VIRTUAL_WIDTH - textWidth) / 2, VIRTUAL_HEIGHT - 10, 10, Fade(WHITE, 0.5f));

        EndTextureMode();

        // **Draw Render Texture to Screen**
        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle sourceRec = {0, 0, VIRTUAL_WIDTH, -VIRTUAL_HEIGHT};
        DrawTexturePro(target.texture, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
        EndDrawing();
    }

    // Cleanup resources
    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}