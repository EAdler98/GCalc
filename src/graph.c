#include "graph.h"
#include "raymath.h"
#include <math.h>
#include <stddef.h>
#include <float.h>

const float STEP_SCALE=.2f; //The smaller the value, the smoother the functions are (slower performence). 
static Color grid_color = { 58, 68, 80, 255 };
static Color label_color = { 170, 184, 196, 255 };
static Color axis_color = { 223, 230, 237, 255 };
static const float HOVER_PICK_RADIUS_PX = 14.0f;

static float point_segment_distance_sq(Vector2 p, Vector2 a, Vector2 b)
{
    Vector2 ab = Vector2Subtract(b, a);
    float ab_len_sq = ab.x * ab.x + ab.y * ab.y;
    if (ab_len_sq <= 0.0f) {
        // Degenerate segment: treat it as a single point.
        Vector2 ap = Vector2Subtract(p, a);
        return ap.x * ap.x + ap.y * ap.y;
    }

    Vector2 ap = Vector2Subtract(p, a);
    // Project p onto the segment, then clamp so the closest point stays on it.
    float t = (ap.x * ab.x + ap.y * ab.y) / ab_len_sq;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    Vector2 closest = Vector2Add(a, Vector2Scale(ab, t));
    Vector2 delta = Vector2Subtract(p, closest);
    return delta.x * delta.x + delta.y * delta.y;
}

void set_graph_dark_mode(bool enabled)
{
    if (enabled) {
        grid_color = (Color){ 58, 68, 80, 255 };
        label_color = (Color){ 170, 184, 196, 255 };
        axis_color = (Color){ 223, 230, 237, 255 };
    } else {
        grid_color = (Color){ 200, 200, 200, 255 };
        label_color = DARKGRAY;
        axis_color = BLACK;
    }
}

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

    // vertical lines + X-axis labels
    int x_start = (int)floorf(tl.x / scale / grid_step) * grid_step;
    int x_end   = (int)ceilf (br.x / scale / grid_step) * grid_step;
    for (int i = x_start; i <= x_end; i += grid_step) {
        if (i != 0)
            DrawLine(i * scale, (int)tl.y, i * scale, (int)br.y, grid_color);
        DrawText(TextFormat("%d", i), i * scale + offset, offset, font_size, label_color);
    }

    // horizontal lines + Y-axis labels
    int y_start = (int)floorf(-br.y / scale / grid_step) * grid_step;
    int y_end   = (int)ceilf (-tl.y / scale / grid_step) * grid_step;
    for (int i = y_start; i <= y_end; i += grid_step) {
        if (i != 0)
            DrawLine((int)tl.x, -i * scale, (int)br.x, -i * scale, grid_color);
        if (i != 0)
            DrawText(TextFormat("%d", i), offset, -i * scale + offset, font_size, label_color);
    }
}

void draw_axes(Camera2D camera)
{
    Vector2 tl = GetScreenToWorld2D((Vector2){ 0,                0                 }, camera);
    Vector2 br = GetScreenToWorld2D((Vector2){ GetScreenWidth(), GetScreenHeight() }, camera);
    DrawLine((int)tl.x, 0, (int)br.x, 0, axis_color);
    DrawLine(0, (int)tl.y, 0, (int)br.y, axis_color);
}

int get_hovered_function_index(Function *f, int count, Camera2D camera, float scale)
{
    if ((f == NULL) || (count <= 0)) return -1;

    Vector2 mouse_screen = GetMousePosition();
    Vector2 left_edge  = GetScreenToWorld2D((Vector2){ 0,                0 }, camera);
    Vector2 right_edge = GetScreenToWorld2D((Vector2){ GetScreenWidth(), 0 }, camera);
    float start_x = left_edge.x / scale;
    float end_x   = right_edge.x / scale;
    float step    = STEP_SCALE / (camera.zoom * scale);
    if (step <= 0.0f) step = 0.01f;

    int hovered = -1;
    float best_distance_sq = FLT_MAX;

    for (int i = 0; i < count; i++) {
        if (f[i].tokens == NULL) continue;

        // Give thicker curves a slightly larger hover target.
        float pick_radius_px = HOVER_PICK_RADIUS_PX + f[i].thickness * 0.5f;
        float pick_radius_sq = pick_radius_px * pick_radius_px;
        float prev_x = start_x;
        double prev_y = evaluate_postfix(f[i].tokens, prev_x);

        for (float x = start_x + step; x <= end_x; x += step) {
            double y = evaluate_postfix(f[i].tokens, x);
            if (!isnan(y) && !isnan(prev_y) && isfinite(y) && isfinite(prev_y)) {
                bool is_pole = false;
                if (prev_y * y < 0) {
                    // Reuse the same pole test as drawing so hover does not snap
                    // onto the fake bridge across an asymptote.
                    double y_mid = evaluate_postfix(f[i].tokens, (prev_x + x) * 0.5f);
                    double lo = prev_y < y ? prev_y : y;
                    double hi = prev_y < y ? y : prev_y;
                    is_pole = isnan(y_mid) || !isfinite(y_mid) || y_mid < lo || y_mid > hi;
                }

                if (!is_pole) {
                    Vector2 p0 = GetWorldToScreen2D((Vector2){ prev_x * scale, (float)-prev_y * scale }, camera);
                    Vector2 p1 = GetWorldToScreen2D((Vector2){ x * scale,      (float)-y      * scale }, camera);
                    // Compare in screen space so "near the line" feels consistent
                    // even when the curve is steep in world coordinates.
                    float distance_sq = point_segment_distance_sq(mouse_screen, p0, p1);
                    if (distance_sq <= pick_radius_sq && distance_sq < best_distance_sq) {
                        best_distance_sq = distance_sq;
                        hovered = i;
                    }
                }
            }

            prev_x = x;
            prev_y = y;
        }
    }

    return hovered;
}



void draw_intersections(Function *f, int count, Camera2D camera, float scale, IntersectMode mode)
{
    if (mode == INTERSECT_NONE) return;
    int hovered_function = -1;
    if (mode == INTERSECT_HOVER) {
        // In hover mode, only show crossings that involve the function nearest
        // to the cursor.
        hovered_function = get_hovered_function_index(f, count, camera, scale);
        if (hovered_function < 0) return;
    }

    Vector2 left_edge  = GetScreenToWorld2D((Vector2){0,                0}, camera);
    Vector2 right_edge = GetScreenToWorld2D((Vector2){GetScreenWidth(), 0}, camera);
    float start_x = left_edge.x / scale;
    float end_x   = right_edge.x / scale;
    float step = STEP_SCALE / (camera.zoom * scale);
    if (step <= 0) step = 0.01f;

    float r         = 5.0f  / camera.zoom;
    float font_size = 20.0f / camera.zoom;
    float pad       = 3.0f  / camera.zoom;

    for (int a = 0; a < count - 1; a++) {
        if (!f[a].tokens) continue;
        for (int b = a + 1; b < count; b++) {
            if (!f[b].tokens) continue;
            if (hovered_function >= 0 && a != hovered_function && b != hovered_function) continue;

            double prev_diff = evaluate_postfix(f[a].tokens, start_x)
                             - evaluate_postfix(f[b].tokens, start_x);

            for (float x = start_x + step; x <= end_x; x += step) {
                double ya = evaluate_postfix(f[a].tokens, x);
                double yb = evaluate_postfix(f[b].tokens, x);
                if (isnan(ya) || isnan(yb)) { prev_diff = NAN; continue; }

                double diff = ya - yb;

                if (!isnan(prev_diff) && prev_diff * diff < 0) {
                    // bisect to find precise crossing
                    float  lo = x - step, hi = x;
                    double diff_lo = prev_diff;
                    for (int k = 0; k < 50; k++) {
                        float  mid = (lo + hi) * 0.5f;
                        double dya = evaluate_postfix(f[a].tokens, mid);
                        double dyb = evaluate_postfix(f[b].tokens, mid);
                        if (isnan(dya) || isnan(dyb)) break;
                        double dm = dya - dyb;
                        if (diff_lo * dm <= 0) hi = mid;
                        else { lo = mid; diff_lo = dm; }
                    }
                    float  ix = (lo + hi) * 0.5f;
                    double iy = evaluate_postfix(f[a].tokens, ix);
                    if (isnan(iy)) { prev_diff = diff; continue; }

                    float wx = ix * scale;
                    float wy = (float)-iy * scale;

                    // dot
                    DrawCircleV((Vector2){wx, wy}, r, WHITE);
                    DrawCircleLinesV((Vector2){wx, wy}, r, label_color);

                    // label background + text
                    const char *label = TextFormat("(%.2f, %.2f)", (double)ix, iy);
                    Vector2 tsz = MeasureTextEx(GetFontDefault(), label, font_size, 1);
                    Rectangle bg = { wx + r + pad, wy - tsz.y - pad,
                                     tsz.x + pad * 2, tsz.y + pad * 2 };
                    DrawRectangleRec(bg, (Color){0, 0, 0, 160});
                    DrawTextEx(GetFontDefault(), label,
                               (Vector2){bg.x + pad, bg.y + pad},
                               font_size, 1, WHITE);
                }
                prev_diff = diff;
            }
        }
    }
}

void draw_function(Function f, Camera2D camera, float scale)
{
    if (f.tokens == NULL) return;

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
            // Pole detection: when sign flips, sample the midpoint.
            // If the midpoint is NaN or jumps outside [min(y0,y1), max(y0,y1)],
            // the function diverged through ±inf — skip the segment.
            bool is_pole = false;
            if (prev_y * y < 0) {
                double y_mid = evaluate_postfix(f.tokens, (prev_x + x) * 0.5f);
                double lo = prev_y < y ? prev_y : y;
                double hi = prev_y < y ? y : prev_y;
                is_pole = isnan(y_mid) || y_mid < lo || y_mid > hi;
            }

            float wy0 = (float)-prev_y * scale;
            float wy1 = (float)-y      * scale;
            if (!is_pole) {
                bool both_above = wy0 < world_y_min && wy1 < world_y_min;
                bool both_below = wy0 > world_y_max && wy1 > world_y_max;
                if (!both_above && !both_below) {
                    Vector2 p0 = { prev_x * scale, wy0 };
                    Vector2 p1 = { x      * scale, wy1 };
                    DrawLineEx(p0, p1, f.thickness / camera.zoom, f.color);
                }
            } else {
                // Each branch gets a vertical stub to the screen boundary so it
                // always appears to shoot off the edge rather than being cut short.
                float w = 2.0f / camera.zoom;
                float edge0 = (prev_y < 0) ? world_y_max : world_y_min;
                float edge1 = (y      > 0) ? world_y_min : world_y_max;
                DrawLineEx((Vector2){ prev_x * scale, wy0 }, (Vector2){ prev_x * scale, edge0 }, w, f.color);
                DrawLineEx((Vector2){ x      * scale, wy1 }, (Vector2){ x      * scale, edge1 }, w, f.color);
            }
        }
        prev_x = x;
        prev_y = y;
    }
}
