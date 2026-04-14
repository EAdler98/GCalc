#include "graph.h"
#include "raymath.h"
#include <math.h>

const float STEP_SCALE=5.0f; //The smaller the value, the smoother the functions are (slower performence). 
void update_camera_smooth(Camera2D *camera, Vector2 *target_pos, float *target_zoom)
{
    const float smooth = 0.1f;

    // panning: right or middle mouse button
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        target_pos->x -= delta.x / camera->zoom;
        target_pos->y -= delta.y / camera->zoom;
    }

    // zoom toward cursor
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 world_before = GetScreenToWorld2D(GetMousePosition(), *camera);
        *target_zoom += wheel * (*target_zoom * 0.1f);
        if (*target_zoom <     0.1f) *target_zoom =     0.1f;
        if (*target_zoom > 10000.0f) *target_zoom = 10000.0f;

        // keep the point under the cursor fixed
        Vector2 mouse = GetMousePosition();
        target_pos->x = world_before.x - (mouse.x - camera->offset.x) / *target_zoom;
        target_pos->y = world_before.y - (mouse.y - camera->offset.y) / *target_zoom;
    }

    // lerp toward targets
    camera->zoom     = Lerp(camera->zoom,     *target_zoom,   smooth);
    camera->target.x = Lerp(camera->target.x, target_pos->x, 0.2f);
    camera->target.y = Lerp(camera->target.y, target_pos->y, 0.2f);

    // snap when close enough to avoid micro-jitter
    if (fabsf(camera->zoom     - *target_zoom)   < 0.001f) camera->zoom     = *target_zoom;
    if (fabsf(camera->target.x - target_pos->x)  < 0.001f) camera->target.x = target_pos->x;
    if (fabsf(camera->target.y - target_pos->y)  < 0.001f) camera->target.y = target_pos->y;
}

void draw_grid(Camera2D camera, float scale)
{
    Vector2 tl = GetScreenToWorld2D((Vector2){ 0,                0                 }, camera);
    Vector2 br = GetScreenToWorld2D((Vector2){ GetScreenWidth(), GetScreenHeight() }, camera);

    // pick a "nice" step so we get roughly 10-15 labels across the screen
    float visible_width = (br.x - tl.x) / scale;
    static const int nice[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000 };
    int grid_step = 1;
    for (int k = 0; k < 10; k++) {
        grid_step = nice[k];
        if (visible_width / grid_step <= 15) break;
    }

    float font_size = 20.0f / camera.zoom;
    float offset    = font_size * 0.3f;

    Color grid_color = (Color){ 200, 200, 200, 255 }; // light gray

    // vertical lines + X-axis labels
    int x_start = (int)floorf(tl.x / scale / grid_step) * grid_step;
    int x_end   = (int)ceilf (br.x / scale / grid_step) * grid_step;
    for (int i = x_start; i <= x_end; i += grid_step) {
        if (i != 0)
            DrawLine(i * scale, (int)tl.y, i * scale, (int)br.y, grid_color);
        DrawText(TextFormat("%d", i), i * scale + offset, offset, font_size, DARKGRAY);
    }

    // horizontal lines + Y-axis labels
    int y_start = (int)floorf(-br.y / scale / grid_step) * grid_step;
    int y_end   = (int)ceilf (-tl.y / scale / grid_step) * grid_step;
    for (int i = y_start; i <= y_end; i += grid_step) {
        if (i != 0)
            DrawLine((int)tl.x, -i * scale, (int)br.x, -i * scale, grid_color);
        if (i != 0)
            DrawText(TextFormat("%d", i), offset, -i * scale + offset, font_size, DARKGRAY);
    }
}

void draw_axes(Camera2D camera)
{
    Vector2 tl = GetScreenToWorld2D((Vector2){ 0,                0                 }, camera);
    Vector2 br = GetScreenToWorld2D((Vector2){ GetScreenWidth(), GetScreenHeight() }, camera);
    DrawLine((int)tl.x, 0, (int)br.x, 0, BLACK);
    DrawLine(0, (int)tl.y, 0, (int)br.y, BLACK);
}

void draw_function(Function f, Camera2D camera, float scale)
{
    Vector2 left_edge  = GetScreenToWorld2D((Vector2){ 0,                0 }, camera);
    Vector2 right_edge = GetScreenToWorld2D((Vector2){ GetScreenWidth(), 0 }, camera);

    float start_x = left_edge.x  / scale;
    float end_x   = right_edge.x / scale;
    float step    = STEP_SCALE / (camera.zoom * scale);
    if (step <= 0) step = 0.01f;

    // visible world-Y range (top has smaller world-Y because Y axis is flipped)
    Vector2 screen_tl = GetScreenToWorld2D((Vector2){0,                0                 }, camera);
    Vector2 screen_br = GetScreenToWorld2D((Vector2){GetScreenWidth(), GetScreenHeight() }, camera);
    float world_y_min = screen_tl.y;
    float world_y_max = screen_br.y;

    float  prev_x = start_x;
    double prev_y = evaluate_postfix(f.tokens, prev_x);

    for (float x = start_x + step; x <= end_x; x += step) {
        double y = evaluate_postfix(f.tokens, x);
        if (!isnan(y) && !isnan(prev_y)) {
            float wy0 = (float)-prev_y * scale;
            float wy1 = (float)-y      * scale;
            // skip segment only if both endpoints are outside the same side
            bool both_above = wy0 < world_y_min && wy1 < world_y_min;
            bool both_below = wy0 > world_y_max && wy1 > world_y_max;
            if (!both_above && !both_below) {
                Vector2 p0 = { prev_x * scale, wy0 };
                Vector2 p1 = { x      * scale, wy1 };
                DrawLineEx(p0, p1, 2.0f / camera.zoom, f.color);
            }
        }
        prev_x = x;
        prev_y = y;
    }
}
