#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Define constants for maximum limits
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

// Helper function to get touch position by ID
Vector2 GetTouchPositionByID(int id) {
    for (int i = 0; i < GetTouchPointCount(); i++) {
        if (GetTouchPointId(i) == id) {
            return GetTouchPosition(i);
        }
    }
    return (Vector2){-1, -1}; // Invalid position
}

// Function to calculate shortest angle between two angles
float ShortestAngle(float from, float to) {
    float diff = fmodf(to - from, 360.0f);
    if (diff > 180.0f) diff -= 360.0f;
    if (diff < -180.0f) diff += 360.0f;
    return diff;
}

int main(void) {
    // Enable resizable window
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1920, 1080, "Fun Internet"); // Initial size, will adjust on resize
    SetTargetFPS(60);

    // Define constant sizes for game objects (in pixels, not scaled with window)
    const float shipSize = 20.0f;       // Size of the spaceship
    const float enemyRadius = 10.0f;    // Radius of enemies
    const float laserLength = 10.0f;    // Length of lasers

    // Initialize game objects
    Spaceship player = {0};
    Laser lasers[MAX_LASERS] = {0};
    Enemy enemies[MAX_ENEMIES] = {0};
    Particle particles[MAX_PARTICLES] = {0};

    // Game state variables
    int score = 0;
    bool playerExploded = false;

    // Random seed
    srand(time(NULL));

    // Touch control variables
    int prevTouchCount = 0;
    int prevTouchIDs[10] = {0};
    int joystickTouchID = -1;
    int fireTouchID = -1;
    bool fireTriggered = false;

    // Variables to track previous window size for scaling positions
    static int prevWidth = 0;
    static int prevHeight = 0;

    // Flag for first frame initialization
    bool firstFrame = true;

    // Main game loop
    while (!WindowShouldClose()) {
        // Get current screen dimensions
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        // Initialize player position on first frame or after reset
        if (firstFrame) {
            player.position = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
            player.rotation = 0.0f;
            player.speed = 0.0f;
            firstFrame = false;
        }

        // Scale positions of all objects when window is resized
        if (prevWidth != 0 && prevHeight != 0 && (prevWidth != screenWidth || prevHeight != screenHeight)) {
            float scaleX = (float)screenWidth / prevWidth;
            float scaleY = (float)screenHeight / prevHeight;

            player.position.x *= scaleX;
            player.position.y *= scaleY;

            for (int i = 0; i < MAX_LASERS; i++) {
                if (lasers[i].active) {
                    lasers[i].position.x *= scaleX;
                    lasers[i].position.y *= scaleY;
                }
            }

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    enemies[i].position.x *= scaleX;
                    enemies[i].position.y *= scaleY;
                }
            }

            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life > 0) {
                    particles[i].position.x *= scaleX;
                    particles[i].position.y *= scaleY;
                }
            }
        }
        prevWidth = screenWidth;
        prevHeight = screenHeight;

        // Calculate font size for scaling text (except for fixed-size text)
        int fontSize = (int)(screenWidth / 50.0f);
        if (fontSize < 10) fontSize = 10; // Minimum font size

        // Define fixed font sizes for specific texts
        const int fixedFontSizeSocial = 10; // For "X kirbara2000"
        const int fixedFontSizeTryAgain = 20; // For "Nah, I'd Win"

        // Top text
        const char* titleText = "RAGE AGAINST THE DYING INTERNET";
        int titleWidth = MeasureText(titleText, fontSize);
        int titleX = (screenWidth - titleWidth) / 2;
        int titleY = 10;

        // Subtext
        const char* subText = "Make your own website, fill with ur SOUL & fuck the SLOP!";
        int subWidth = MeasureText(subText, fontSize / 2);
        int subX = (screenWidth - subWidth) / 2;
        int subY = titleY + fontSize + 10;

        // Control text
        const char* controlText = "ARROW and SPACE to control your Spaceship";
        int controlWidth = MeasureText(controlText, fontSize / 2);
        int controlX = (screenWidth - controlWidth) / 2;
        int controlY = subY + fontSize / 2 + 10;

        // Social media button
        int buttonWidth = 100;
        int buttonHeight = 20;
        Rectangle buttonRect = {(screenWidth - buttonWidth) / 2, controlY + fontSize / 2 + 10, buttonWidth, buttonHeight};

        // Bottom text split into two lines
        const char* bottomTextLine1 = "This site is written in C, Raylib, Wasm, & Grok.";
        const char* bottomTextLine2 = "Techzz is crazyy. U can do THINGS!";
        int bottomWidthLine1 = MeasureText(bottomTextLine1, fontSize / 2);
        int bottomWidthLine2 = MeasureText(bottomTextLine2, fontSize / 2);
        int bottomXLine1 = (screenWidth - bottomWidthLine1) / 2;
        int bottomXLine2 = (screenWidth - bottomWidthLine2) / 2;
        int bottomYLine1 = screenHeight - (fontSize / 2 + 5 + fontSize / 2 + 10);
        int bottomYLine2 = bottomYLine1 + fontSize / 2 + 5;

        // Score or game over text (moved up to avoid overlap)
        int scoreY = screenHeight - (fontSize + fontSize / 2 + 40);

        // "Try Again" button
        Rectangle tryAgainButton = {(screenWidth - 200) / 2, screenHeight / 2, 200, 50};

        // Touch controller positions
        float controllerSize = fminf(screenWidth, screenHeight) * 0.1f;
        float joystickRadius = controllerSize;
        float fireButtonRadius = controllerSize;

        // Define joystick and fire button positions
        Vector2 joystickCenter;
        Vector2 fireButtonCenter;

        if (screenWidth > screenHeight) { // Horizontal
            joystickCenter = (Vector2){screenWidth * 0.1f, screenHeight * 0.8f};
            fireButtonCenter = (Vector2){screenWidth * 0.9f, screenHeight * 0.8f};
        } else { // Vertical
            joystickCenter = (Vector2){screenWidth * 0.25f, screenHeight * 0.8f};
            fireButtonCenter = (Vector2){screenWidth * 0.75f, screenHeight * 0.8f};
        }

        // Touch control logic
        int touchCount = GetTouchPointCount();
        int currentTouchIDs[10] = {0};
        for (int i = 0; i < touchCount; i++) {
            currentTouchIDs[i] = GetTouchPointId(i);
        }

        // Detect new touches
        for (int i = 0; i < touchCount; i++) {
            int touchID = currentTouchIDs[i];
            bool isNew = true;
            for (int j = 0; j < prevTouchCount; j++) {
                if (prevTouchIDs[j] == touchID) {
                    isNew = false;
                    break;
                }
            }
            if (isNew) {
                Vector2 touchPos = GetTouchPosition(i);
                if (CheckCollisionPointCircle(touchPos, joystickCenter, joystickRadius)) {
                    if (joystickTouchID == -1) {
                        joystickTouchID = touchID;
                    }
                } else if (CheckCollisionPointCircle(touchPos, fireButtonCenter, fireButtonRadius)) {
                    if (fireTouchID == -1) {
                        fireTouchID = touchID;
                        fireTriggered = true;
                    }
                }
            }
        }

        // Update joystick control
        Vector2 joystickTouchPos = joystickCenter;
        if (joystickTouchID != -1) {
            Vector2 touchPos = GetTouchPositionByID(joystickTouchID);
            if (touchPos.x != -1) {
                float dx = touchPos.x - joystickCenter.x;
                float dy = touchPos.y - joystickCenter.y;
                float distance = sqrtf(dx * dx + dy * dy);

                if (distance > joystickRadius) {
                    dx = dx * joystickRadius / distance;
                    dy = dy * joystickRadius / distance;
                    distance = joystickRadius;
                }

                joystickTouchPos.x = joystickCenter.x + dx;
                joystickTouchPos.y = joystickCenter.y + dy;

                if (distance > joystickRadius * 0.2f) {
                    float targetRotation = atan2f(dy, dx) * RAD2DEG;
                    float angleDiff = ShortestAngle(player.rotation, targetRotation);
                    float rotationSpeed = 5.0f;
                    float rotationStep = fminf(fabsf(angleDiff), rotationSpeed) * (angleDiff > 0 ? 1.0f : -1.0f);
                    player.rotation += rotationStep;
                    player.rotation = fmodf(player.rotation, 360.0f);
                    if (player.rotation < 0) player.rotation += 360.0f;
                }

                float normalizedDistance = distance / joystickRadius;
                float targetSpeed = normalizedDistance * 5.0f;
                float acceleration = 0.05f;
                if (player.speed < targetSpeed) {
                    player.speed += acceleration;
                    if (player.speed > targetSpeed) player.speed = targetSpeed;
                } else if (player.speed > targetSpeed) {
                    player.speed -= acceleration;
                    if (player.speed < targetSpeed) player.speed = targetSpeed;
                }
                player.speed = fmaxf(0.0f, fminf(player.speed, 5.0f));
            } else {
                joystickTouchID = -1;
                player.speed -= 0.03f;
                player.speed = fmaxf(0.0f, player.speed);
            }
        } else {
            player.speed -= 0.03f;
            player.speed = fmaxf(0.0f, player.speed);
        }

        // Update fire button control
        if (fireTouchID != -1) {
            Vector2 touchPos = GetTouchPositionByID(fireTouchID);
            if (touchPos.x == -1) {
                fireTouchID = -1;
            }
        }

        // Update previous touch count and IDs
        prevTouchCount = touchCount;
        for (int i = 0; i < touchCount; i++) {
            prevTouchIDs[i] = currentTouchIDs[i];
        }

        // Game logic
        if (!playerExploded) {
            // Keyboard controls
            if (IsKeyDown(KEY_RIGHT)) player.rotation += 5.0f;
            if (IsKeyDown(KEY_LEFT)) player.rotation -= 5.0f;
            if (IsKeyDown(KEY_UP)) player.speed += 0.1f;
            if (IsKeyDown(KEY_DOWN)) player.speed -= 0.1f;
            player.speed = fmaxf(0.0f, fminf(player.speed, 5.0f));

            // Player movement
            float rad = player.rotation * DEG2RAD;
            player.position.x += player.speed * cosf(rad);
            player.position.y += player.speed * sinf(rad);
            player.position.x = (player.position.x > screenWidth) ? 0 : (player.position.x < 0) ? screenWidth : player.position.x;
            player.position.y = (player.position.y > screenHeight) ? 0 : (player.position.y < 0) ? screenHeight : player.position.y;

            // Shoot lasers
            if (fireTriggered || IsKeyPressed(KEY_SPACE)) {
                for (int i = 0; i < MAX_LASERS; i++) {
                    if (!lasers[i].active) {
                        lasers[i].position = (Vector2){player.position.x + laserLength * cosf(rad), player.position.y + laserLength * sinf(rad)};
                        lasers[i].rotation = player.rotation;
                        lasers[i].speed = 10.0f;
                        lasers[i].active = true;
                        break;
                    }
                }
                fireTriggered = false;
            }

            // Update lasers
            for (int i = 0; i < MAX_LASERS; i++) {
                if (lasers[i].active) {
                    float laserRad = lasers[i].rotation * DEG2RAD;
                    lasers[i].position.x += lasers[i].speed * cosf(laserRad);
                    lasers[i].position.y += lasers[i].speed * sinf(laserRad);
                    if (lasers[i].position.x < 0 || lasers[i].position.x > screenWidth ||
                        lasers[i].position.y < 0 || lasers[i].position.y > screenHeight) {
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
                        enemies[i].position.x = rand() % screenWidth;
                        enemies[i].position.y = rand() % screenHeight;
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
                        if (enemies[j].active) {
                            if (CheckCollisionPointCircle(lasers[i].position, enemies[j].position, enemyRadius)) {
                                lasers[i].active = false;
                                enemies[j].active = false;
                                score++;
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
            }

            // Check collision between player and enemies
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    if (CheckCollisionCircles(player.position, shipSize / 2, enemies[i].position, enemyRadius)) {
                        playerExploded = true;
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
        }

        // Handle "Try Again" button and Tab key
        if (playerExploded) {
            bool isTryAgainHovered = CheckCollisionPointRec(GetMousePosition(), tryAgainButton);
            if ((isTryAgainHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_TAB)) {
                player.position = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
                player.rotation = 0.0f;
                player.speed = 0.0f;
                for (int i = 0; i < MAX_LASERS; i++) lasers[i].active = false;
                for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
                score = 0;
                playerExploded = false;
            }
        }

        // Update particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life > 0) {
                particles[i].position.x += particles[i].velocity.x;
                particles[i].position.y += particles[i].velocity.y;
                particles[i].alpha -= 0.01f;
                particles[i].life -= 0.01f;
                if (particles[i].alpha < 0) particles[i].alpha = 0;
            }
        }

        // Create background firework effect
        static float fireworkTimer = 0.0f;
        fireworkTimer += GetFrameTime();
        if (fireworkTimer > 0.1f) {
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].life <= 0) {
                    particles[i].position = (Vector2){rand() % screenWidth, rand() % screenHeight};
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

        // Draw directly to screen
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw fireworks and explosion particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].life > 0) {
                DrawPixelV(particles[i].position, Fade(particles[i].color, particles[i].alpha));
            }
        }

        // Draw player spaceship if not exploded
        if (!playerExploded) {
            float rad = player.rotation * DEG2RAD;
            Vector2 front = {player.position.x + shipSize * cosf(rad), player.position.y + shipSize * sinf(rad)};
            Vector2 backLeft = {player.position.x - shipSize * 0.5f * cosf(rad) + shipSize * 0.5f * sinf(rad), player.position.y - shipSize * 0.5f * sinf(rad) - shipSize * 0.5f * cosf(rad)};
            Vector2 backRight = {player.position.x - shipSize * 0.5f * cosf(rad) - shipSize * 0.5f * sinf(rad), player.position.y - shipSize * 0.5f * sinf(rad) + shipSize * 0.5f * cosf(rad)};
            DrawTriangle(front, backLeft, backRight, WHITE);
        }

        // Draw active lasers
        for (int i = 0; i < MAX_LASERS; i++) {
            if (lasers[i].active) {
                float laserRad = lasers[i].rotation * DEG2RAD;
                Vector2 end = {lasers[i].position.x + laserLength * cosf(laserRad), lasers[i].position.y + laserLength * sinf(laserRad)};
                DrawLineV(lasers[i].position, end, YELLOW);
            }
        }

        // Draw active enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                DrawCircleV(enemies[i].position, enemyRadius, RED);
            }
        }

        // Draw UI
        static float timer = 0.0f;
        timer += GetFrameTime();
        Color colors[] = {BLUE, RED, GREEN, WHITE, MAGENTA};
        int colorIndex = (int)(timer * 2) % 5;
        DrawText(titleText, titleX, titleY, fontSize, colors[colorIndex]);

        DrawText(subText, subX, subY, fontSize / 2, WHITE);
        DrawText(controlText, controlX, controlY, fontSize / 2, Fade(WHITE, 0.5f));

        bool isButtonHovered = CheckCollisionPointRec(GetMousePosition(), buttonRect);
        Color buttonColor = isButtonHovered ? YELLOW : WHITE;
        DrawRectangleRec(buttonRect, buttonColor);
        int buttonTextWidth = MeasureText("X kirbara2000", fixedFontSizeSocial);
        int buttonTextX = buttonRect.x + (buttonRect.width - buttonTextWidth) / 2;
        int buttonTextY = buttonRect.y + (buttonRect.height - fixedFontSizeSocial) / 2;
        DrawText("X kirbara2000", buttonTextX, buttonTextY, fixedFontSizeSocial, BLACK);
        if (isButtonHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            OpenURL("https://x.com/kirbara2000");
        }

        if (!playerExploded) {
            char scoreText[32];
            sprintf(scoreText, "Score: %d", score);
            int scoreWidth = MeasureText(scoreText, fontSize);
            DrawText(scoreText, (screenWidth - scoreWidth) / 2, scoreY, fontSize, YELLOW);
        } else {
            char gameOverText[64];
            sprintf(gameOverText, "IT'S SO OVER. TOTAL: %d", score);
            int gameOverWidth = MeasureText(gameOverText, fontSize);
            DrawText(gameOverText, (screenWidth - gameOverWidth) / 2, scoreY, fontSize, YELLOW);

            // Draw "Nah, I'd Win" button
            bool isTryAgainHovered = CheckCollisionPointRec(GetMousePosition(), tryAgainButton);
            Color tryAgainColor = isTryAgainHovered ? YELLOW : WHITE;
            DrawRectangleRec(tryAgainButton, tryAgainColor);
            int tryAgainTextWidth = MeasureText("Nah, I'd Win", fixedFontSizeTryAgain);
            int tryAgainTextX = tryAgainButton.x + (tryAgainButton.width - tryAgainTextWidth) / 2;
            int tryAgainTextY = tryAgainButton.y + (tryAgainButton.height - fixedFontSizeTryAgain) / 2;
            DrawText("Nah, I'd Win", tryAgainTextX, tryAgainTextY, fixedFontSizeTryAgain, BLACK);

            // Draw "press TAB" text
            const char* pressTabText = "press TAB";
            int pressTabFontSize = fixedFontSizeTryAgain / 2; // Smaller font size
            int pressTabWidth = MeasureText(pressTabText, pressTabFontSize);
            int pressTabX = tryAgainButton.x + (tryAgainButton.width - pressTabWidth) / 2;
            int pressTabY = tryAgainButton.y + tryAgainButton.height + 10; // 10 pixels below button
            DrawText(pressTabText, pressTabX, pressTabY, pressTabFontSize, Fade(WHITE, 0.5f));
        }

        DrawText(bottomTextLine1, bottomXLine1, bottomYLine1, fontSize / 2, Fade(WHITE, 0.5f));
        DrawText(bottomTextLine2, bottomXLine2, bottomYLine2, fontSize / 2, Fade(WHITE, 0.5f));

        // Draw touch controls
        DrawCircleV(joystickCenter, joystickRadius, Fade(WHITE, 0.2f));
        if (joystickTouchID != -1) {
            DrawCircleV(joystickTouchPos, 10, Fade(WHITE, 0.5f));
        }
        DrawCircleV(fireButtonCenter, fireButtonRadius, Fade(RED, 0.2f));

        // Draw custom mouse cursor
        Vector2 mousePos = GetMousePosition();
        DrawCircleV(mousePos, 5.0f, BLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}