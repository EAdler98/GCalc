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
    Token *tokens1 = parse("x^2 + 2*x - 3");
    Token *tokens2 = parse("-x^2 + 3");
    Function * f=calloc(10,sizeof(Function));
    f[0]=(Function){tokens1,RED};
    f[1]=(Function){tokens2,BLUE};
    int fcount=2;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "GCalc");
    SetWindowFocused();
    SetTargetFPS(currentFPS);

    Camera2D camera = {0};
    camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    camera.zoom = 1.0f;
    Vector2 target_pos = camera.target;
    float target_zoom = camera.zoom;
    float idleTimer;
    Textbox tb = {0};
    strncpy(tb.text, "x^2 + 2*x - 3", TEXTBOX_MAX - 1);
    tb.len = strlen(tb.text);

    // to load a custom font:
    //   tb.font = LoadFontEx("resources/myfont.ttf", 32, NULL, 0);
    //   tb.font_size = 22;
    Rectangle tb_bounds = {10, screenHeight - 46, 320, 36};

    while (!WindowShouldClose())
    {

        tb_bounds.y=GetScreenHeight()-46;
        // re-parse when user submits a new expression
        bool isTextBoxActive = textbox_update(&tb, tb_bounds);
        // if (isTextBoxActive)
        // {
        //     free(tokens);
        //     tokens = parse(tb.text);
        // }
        Vector2 mouseDelta = GetMouseDelta();
        float wheel = GetMouseWheelMove();
        int keyPressed = GetKeyPressed();
        

        // (נוסיף גם בדיקה אם תיבת הטקסט לחוצה, כדי שהסמן יהבהב בצורה חלקה)
        bool isUserActive = (IsMouseButtonDown(MOUSE_BUTTON_LEFT)||IsMouseButtonDown(MOUSE_BUTTON_RIGHT)||wheel != 0 || keyPressed != 0 || isTextBoxActive);

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
            if (idleTimer > 1.0f && currentFPS != 15)
            {
                currentFPS = 5;
                SetTargetFPS(currentFPS); // הולכים לישון
            }
        }

        update_camera_smooth(&camera, &target_pos, &target_zoom);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);
        draw_grid(camera, SCALE);
        draw_axes(camera);
        Function * p =f;
        for(int i=0;i<fcount;i++) 
        {
            draw_function(f[i],camera,SCALE);
        }
        EndMode2D();
        textbox_draw(&tb, tb_bounds);
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
