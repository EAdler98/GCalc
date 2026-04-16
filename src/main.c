#include "raylib.h"
#include "parser.h"
#include "graph.h"
#include "ui.h"
#include <stdlib.h>
#include <string.h>

int screenWidth = 1920;
int screenHeight = 1080;
const float SCALE = 20.0f;
int currentFPS = 120;
int main(void)
{
    char * text1=calloc(256,sizeof(char));
    strcpy(text1,"1/x");
    char * text2=calloc(256,sizeof(char));
    strcpy(text2,"-x^2 +3");
    Token *tokens1 = parse(text1);
    Token *tokens2 = parse(text2);
    Function *f = calloc(2, sizeof(Function));
    f[0] = (Function){tokens1, RED};
    f[0].tb.text=text1;
    f[0].tb.len=strlen(text1);
    f[0].tb.cursor=f[0].tb.len;
    f[0].tb.font_size=22;
    f[0].thickness=3;
    f[0].slider = (Slider){ 3.0f, false };
    f[1] = (Function){tokens2, BLUE};
    f[1].tb.text=text2;
    f[1].tb.len=strlen(text2);
    f[1].tb.cursor=f[1].tb.len;
    f[1].tb.font_size=22;
    f[1].thickness=3;
    f[1].slider = (Slider){ 3.0f, false };
    int fcount = 2;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "GCalc");
    SetWindowFocused();
    SetTargetFPS(currentFPS);

    Camera2D camera = {0};
    camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    camera.zoom = 1.0f;
    Vector2 target_pos = camera.target;
    float target_zoom = camera.zoom;
    float idleTimer = 0.0f;

  

    while (!WindowShouldClose())
    {
        bool any_textbox_active = false;

        for (int i = 0; i < fcount; i++)
        {
            any_textbox_active = any_textbox_active || f[i].tb.active;
        }

        float wheel = GetMouseWheelMove();
        bool isUserActive = (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ||
                             IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || wheel != 0 || any_textbox_active ||
                             IsKeyPressed(KEY_F11));

        if (isUserActive)
        {
            idleTimer = 0.0f;
            if (currentFPS != 60)
            {
                currentFPS = 60;
                SetTargetFPS(currentFPS);
            }
        }
        else
        {
            idleTimer += GetFrameTime();
            if (idleTimer > 1.0f && currentFPS != 20)
            {
                currentFPS = 20;
                SetTargetFPS(currentFPS);
            }
        }

        update_camera_smooth(&camera, &target_pos, &target_zoom);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        draw_grid(camera, SCALE);
        draw_axes(camera);
        for (int i = 0; i < fcount; i++)
        {
            draw_function(f[i], camera, SCALE);
        }
        EndMode2D();

        FunctionPanelResult panel = draw_functions_tbs(f, fcount, 8);

        DrawText(TextFormat("Zoom: %.2f", camera.zoom), 10, 10, 18, DARKGRAY);
        EndDrawing();

        if (IsKeyPressed(KEY_F11))
        {
            ToggleFullscreen();
        }

        if (panel.to_reparse >= 0) {
            free(f[panel.to_reparse].tokens);
            f[panel.to_reparse].tokens = parse(f[panel.to_reparse].tb.text);
        }

        if (panel.to_remove >= 0) {
            free(f[panel.to_remove].tokens);
            free(f[panel.to_remove].tb.text);
            memmove(&f[panel.to_remove], &f[panel.to_remove + 1], (fcount - panel.to_remove - 1) * sizeof(Function));
            fcount--;
        }
        if (panel.do_add) {
            f = realloc(f, (fcount + 1) * sizeof(Function));
            f[fcount] = (Function){0};
            char *text = calloc(256, sizeof(char));
            text[0] = '\0';
            f[fcount].tokens       = parse(text);
            f[fcount].color        = palette[fcount % 8];
            f[fcount].tb.text      = text;
            f[fcount].tb.len       = 0;
            f[fcount].tb.cursor    = 0;
            f[fcount].tb.font_size = 22;
            f[fcount].tb.active    = true;
            f[fcount].thickness    = 3.0f;
            f[fcount].slider       = (Slider){ 3.0f, false };
            fcount++;
        }
    }

    CloseWindow();
    return 0;
}
