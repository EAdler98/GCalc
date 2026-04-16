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
    float idleTimer;
    // Textbox tb = {0};
    // strncpy(tb.text, "x^2 + 2*x - 3", TEXTBOX_MAX - 1);
    // tb.len = strlen(tb.text);

    // to load a custom font:
    //   tb.font = LoadFontEx("resources/myfont.ttf", 32, NULL, 0);
    //   tb.font_size = 22;

    const int TB_X = 10, TB_W = 320, TB_H = 36, TB_MARGIN = 10, TB_PAD = 4;

    while (!WindowShouldClose())
    {

        bool isTextBoxActive = false;
        bool is_one_tb_active;
        // re-parse when user submits a new expression
        
        for (int i = 0; i < fcount; i++)
        {
            Rectangle b  = { TB_X, GetScreenHeight() - TB_MARGIN - TB_H - i * (TB_H + TB_PAD), TB_W, TB_H };
            Rectangle sb = { TB_X + TB_W + 8, b.y, 120, TB_H };
            is_one_tb_active = textbox_update(&f[i].tb, b);
            isTextBoxActive |= is_one_tb_active;
            if (is_one_tb_active)
            {
                free(f[i].tokens);
                f[i].tokens = parse(f[i].tb.text);
            }
            if (slider_update(&f[i].slider, sb))
                f[i].thickness = f[i].slider.value;
        }

        Vector2 mouseDelta = GetMouseDelta();
        float wheel = GetMouseWheelMove();
        int keyPressed = GetKeyPressed();

        // (נוסיף גם בדיקה אם תיבת הטקסט לחוצה, כדי שהסמן יהבהב בצורה חלקה)
        bool isUserActive = (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || wheel != 0 || keyPressed != 0 || isTextBoxActive);

        // 2. עדכון הטיימר וה-FPS
        if (isUserActive)
        {
            idleTimer = 0.0f; // איפוס הטיימר
            if (currentFPS != 60)
            {
                currentFPS = 60;
                SetTargetFPS(currentFPS); // מתעוררים!
            }
        }
        else
        {
            idleTimer += GetFrameTime(); // הוספת הזמן שעבר מאז הפריים הקודם

            // אם עברו 2 שניות בלי תזוזה, נכנסים למצב שינה
            if (idleTimer > 1.0f && currentFPS != 20)
            {
                currentFPS = 20;
                SetTargetFPS(currentFPS); // הולכים לישון
            }
        }

        update_camera_smooth(&camera, &target_pos, &target_zoom);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        draw_grid(camera, SCALE);
        draw_axes(camera);
        Function *p = f;
        for (int i = 0; i < fcount; i++)
        {
            draw_function(f[i], camera, SCALE);
        }
        EndMode2D();
        Rectangle tb_start = { TB_X, GetScreenHeight() - TB_MARGIN - TB_H, TB_W, TB_H };
        draw_functions_tbs(f, fcount, tb_start, TB_PAD);
        
        DrawText(TextFormat("Zoom: %.2f", camera.zoom), 10, 10, 18, DARKGRAY);
        EndDrawing();
        if (IsKeyPressed(KEY_F11))
        {
            ToggleFullscreen();
        }
    }

    CloseWindow();
    return 0;
}
