#include "raylib.h"

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Fun Internet");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(400, 300);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        int width = GetScreenWidth();
        int height = GetScreenHeight();

        int mainFontSize = 40;
        int secondaryFontSize = 20;
        int bottomFontSize = 20;
        int spacing = 20;

        int mainTextHeight = mainFontSize;
        int secondaryTextHeight = secondaryFontSize;
        int bottomTextHeight = bottomFontSize;

        int totalHeight = mainTextHeight + spacing + secondaryTextHeight + spacing + bottomTextHeight;
        int startingY = (height - totalHeight) / 2;

        const char *mainText = "PLEASE BUILD YOUR OWN PERSONAL WEBSITE";
        int mainTextWidth = MeasureText(mainText, mainFontSize);
        int mainX = (width - mainTextWidth) / 2;
        int mainY = startingY;
        DrawText(mainText, mainX, mainY, mainFontSize, WHITE);

        const char *secondaryText = "this site is written in C, yes, you can just do things!";
        int secondaryTextWidth = MeasureText(secondaryText, secondaryFontSize);
        int secondaryX = (width - secondaryTextWidth) / 2;
        int secondaryY = mainY + mainTextHeight + spacing;
        DrawText(secondaryText, secondaryX, secondaryY, secondaryFontSize, WHITE);

        const char *madeBy = "still experimenting by ";
        const char *linkText = "kirbara2000";
        const char *andGrok = " & groky :3";
        int madeByWidth = MeasureText(madeBy, bottomFontSize);
        int linkWidth = MeasureText(linkText, bottomFontSize);
        int andGrokWidth = MeasureText(andGrok, bottomFontSize);
        int totalBottomWidth = madeByWidth + linkWidth + andGrokWidth;
        int bottomX = (width - totalBottomWidth) / 2;
        int bottomY = secondaryY + secondaryTextHeight + spacing;

        DrawText(madeBy, bottomX, bottomY, bottomFontSize, WHITE);

        int linkX = bottomX + madeByWidth;
        DrawText(linkText, linkX, bottomY, bottomFontSize, WHITE);

        int andGrokX = linkX + linkWidth;
        DrawText(andGrok, andGrokX, bottomY, bottomFontSize, WHITE);

        Vector2 mousePos = GetMousePosition();
        if (mousePos.x >= linkX && mousePos.x <= linkX + linkWidth &&
            mousePos.y >= bottomY && mousePos.y <= bottomY + bottomFontSize) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                OpenURL("https://x.com/kirbara2000");
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}