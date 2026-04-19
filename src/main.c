#include "raylib.h"
#include "parser.h"
#include "graph.h"
#include "ui.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

int screenWidth = 1920;
int screenHeight = 1080;
const float SCALE = 20.0f;
int currentFPS = 120;
static const Color DARK_BACKGROUND = { 13, 17, 23, 255 };
static const Color LIGHT_BACKGROUND = RAYWHITE;
static const Color DARK_HUD_TEXT = { 230, 237, 243, 255 };
static const Color LIGHT_HUD_TEXT = DARKGRAY;

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
    f[1] = (Function){tokens2, BLUE};
    f[1].tb.text=text2;
    f[1].tb.len=strlen(text2);
    f[1].tb.cursor=f[1].tb.len;
    f[1].tb.font_size=22;
    f[1].thickness=3;
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
    bool dark_mode = true;
    Slider thickness_slider = { 3.0f, false };
    IntersectMode intersect_mode = INTERSECT_ALL;

    ui_set_dark_mode(dark_mode);
    set_graph_dark_mode(dark_mode);

  

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
        ClearBackground(dark_mode ? DARK_BACKGROUND : LIGHT_BACKGROUND);
        BeginMode2D(camera);
        draw_grid(camera, SCALE);
        draw_axes(camera);
        for (int i = 0; i < fcount; i++)
        {
            draw_function(f[i], camera, SCALE);
        }
        draw_intersections(f, fcount, camera, SCALE, intersect_mode);
        EndMode2D();

        FunctionPanelResult panel = draw_functions_tbs(f, fcount, 8);
        Rectangle theme_button = { 10, 34, 130, 28 };
        if (button(theme_button, dark_mode ? "Light Mode" : "Dark Mode")) {
            dark_mode = !dark_mode;
            ui_set_dark_mode(dark_mode);
            set_graph_dark_mode(dark_mode);
        }
        DrawText("Thickness", 10, 68, 16, dark_mode ? DARK_HUD_TEXT : LIGHT_HUD_TEXT);
        if (slider_update(&thickness_slider, (Rectangle){ 10, 88, 130, 24 })) {
            for (int i = 0; i < fcount; i++) f[i].thickness = thickness_slider.value;
        }
        DrawText("Crossings", 10, 118, 16, dark_mode ? DARK_HUD_TEXT : LIGHT_HUD_TEXT);
        {
            int im = (int)intersect_mode;
            toggle_group((Rectangle){ 10, 138, 130, 24 }, "All;Off", &im);
            intersect_mode = (IntersectMode)im;
        }

        DrawText(TextFormat("Zoom: %.2f", camera.zoom), 10, 10, 18, dark_mode ? DARK_HUD_TEXT : LIGHT_HUD_TEXT);
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
            f[fcount].thickness    = thickness_slider.value;
            fcount++;
        }
    }

    CloseWindow();
    return 0;
}
