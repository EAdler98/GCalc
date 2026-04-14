#ifndef GRAPH_H
#define GRAPH_H

#include "raylib.h"
#include "parser.h"
#include "ui.h"

typedef struct {
    Token *tokens;
    Color  color;
    Textbox tb;

} Function;

void update_camera_smooth(Camera2D *camera, Vector2 *target_pos, float *target_zoom);
void draw_grid(Camera2D camera, float scale);
void draw_axes(Camera2D camera);
void draw_function(Function f, Camera2D camera, float scale);
void draw_functions(Function *f, int count, Rectangle start, int padding);

#endif
