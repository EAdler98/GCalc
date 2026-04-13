/*******************************************************************************************
 *
 *   raylib [core] example - basic window
 *
 *   Example complexity rating: [★☆☆☆] 1/4
 *
 *   Welcome to raylib!
 *
 *   To test examples, just press F6 and execute 'raylib_compile_execute' script
 *   Note that compiled executable is placed in the same folder as .c file
 *
 *   To test the examples on Web, press F6 and execute 'raylib_compile_execute_web' script
 *   Web version of the program is generated in the same folder as .c file
 *
 *   You can find all basic examples on C:\raylib\raylib\examples folder or
 *   raylib official webpage: www.raylib.com
 *
 *   Enjoy using raylib. :)
 *
 *   Example originally created with raylib 1.0, last time updated with raylib 1.0
 *
 *   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
 *   BSD-like license that allows static linking with closed source software
 *
 *   Copyright (c) 2013-2025 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "parser.h"
#include "graph.h"
#include <stdlib.h>

const int   screenWidth  = 1920;
const int   screenHeight = 1080;
const float SCALE        = 20.0f;
int main(void)
{
    

    Token *tokens = parse("x^2 + 2*x - 3");

    InitWindow(screenWidth, screenHeight, "GCalc");
    SetWindowFocused();
    SetTargetFPS(60);

    Camera2D camera  = {0};
    camera.offset    = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.zoom      = 1.0f;
    Vector2 target_pos = camera.target;
    float target_zoom = camera.zoom;

    while (!WindowShouldClose())
    {
        update_camera_smooth(&camera, &target_pos, &target_zoom);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode2D(camera);
                draw_grid(camera, SCALE);
                draw_axes(camera);
                draw_function(tokens, camera, SCALE);
            EndMode2D();
            DrawText(TextFormat("Zoom: %.2f",    camera.zoom),     10, 10, 20, DARKGRAY);
            DrawText(TextFormat("Target: %.1f, %.1f", camera.target.x, camera.target.y), 10, 40, 20, DARKGRAY);
            DrawText(TextFormat("Offset: %.1f, %.1f", camera.offset.x, camera.offset.y), 10, 60, 20, DARKGRAY);
        EndDrawing();
    }

    free(tokens);
    CloseWindow();
    return 0;
}