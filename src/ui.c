#include "ui.h"
#include <string.h>
#include <math.h>


const float min = 1.0f;
const float max = 5.0f;
const float step =0.5f;

bool textbox_update(Textbox *tb, Rectangle bounds)
{
    int prev_len = tb->len;

    // click to focus / unfocus
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        tb->active = CheckCollisionPointRec(GetMousePosition(), bounds);

    if (!tb->active) return false;

    // arrow keys: move cursor
    if (IsKeyPressed(KEY_LEFT)  && tb->cursor > 0)      tb->cursor--;
    if (IsKeyPressed(KEY_RIGHT) && tb->cursor < tb->len) tb->cursor++;
    if (IsKeyPressed(KEY_HOME) ) tb->cursor=0;
    if (IsKeyPressed(KEY_END)) tb->cursor=tb->len;

    // character input: insert at cursor
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && tb->len < TEXTBOX_MAX - 1) {
            memmove(&tb->text[tb->cursor + 1], &tb->text[tb->cursor], tb->len - tb->cursor + 1);
            tb->text[tb->cursor++] = (char)key;
            tb->len++;
        }
        key = GetCharPressed();
    }

    // backspace: delete character before cursor
    if (IsKeyPressed(KEY_BACKSPACE) && tb->cursor > 0) {
        memmove(&tb->text[tb->cursor - 1], &tb->text[tb->cursor], tb->len - tb->cursor + 1);
        tb->cursor--;
        tb->len--;
    }

    return tb->len != prev_len;
}

void textbox_draw(const Textbox *tb, Rectangle b)
{
    int   fs     = tb->font_size > 0 ? tb->font_size : (int)b.height - 10;
    bool  custom = tb->font.texture.id > 0;
    Font  font   = custom ? tb->font : GetFontDefault();
    int   text_y = (int)b.y + ((int)b.height - fs) / 2;

    Color bg     = tb->active ? WHITE    : LIGHTGRAY;
    Color border = tb->active ? DARKBLUE : DARKGRAY;

    DrawRectangleRec(b, bg);
    DrawRectangleLinesEx(b, 2, border);
    DrawTextEx(font, tb->text, (Vector2){ b.x + 6, text_y }, fs, 1, DARKGRAY);

    // blinking cursor at insertion point
    if (tb->active && ((int)(GetTime() * 2) % 2)) {
        char tmp[TEXTBOX_MAX];
        int clen = tb->cursor < TEXTBOX_MAX ? tb->cursor : TEXTBOX_MAX - 1;
        memcpy(tmp, tb->text, clen);
        tmp[clen] = '\0';
        Vector2 measured = MeasureTextEx(font, tmp, fs, 1);
        int cx = (int)b.x + 6 + (int)measured.x;
        DrawLine(cx, (int)b.y + 4, cx, (int)b.y + (int)b.height - 4, DARKGRAY);
    }
}

bool slider_update(Slider *s, Rectangle b)
{
   
    float prev = s->value;
    Vector2 mouse = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, b))
        s->dragging = true;
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        s->dragging = false;

    if (s->dragging) {
        const float pad = 8.0f;
        float t = (mouse.x - (b.x + pad)) / (b.width - pad * 2.0f);
        t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
        float raw = min + t * (max - min);
        s->value = roundf(raw / step) * step;
        s->value = s->value < min ? min : s->value > max ? max : s->value;
    }

    return s->value != prev;
}

void slider_draw(const Slider *s, Rectangle b, Color accent)
{

    const float pad     = 8.0f;
    const float track_x = b.x + pad;
    const float track_w = b.width - pad * 2.0f;
    const float track_y = b.y + b.height * 0.72f;
    const int   thumb_r = 7;

    float t       = (s->value - min) / (max - min);
    float thumb_x = track_x + t * track_w;

    // Background
    DrawRectangleRec(b, (Color){ 230, 230, 230, 255 });
    DrawRectangleLinesEx(b, 1, GRAY);

    // Track (empty)
    DrawRectangle((int)track_x, (int)track_y - 2, (int)track_w, 4, GRAY);
    // Track (filled up to thumb)
    DrawRectangle((int)track_x, (int)track_y - 2, (int)(thumb_x - track_x), 4, accent);

    // Thumb: outer ring then accent fill
    DrawCircle((int)thumb_x, (int)track_y, thumb_r,     WHITE);
    DrawCircle((int)thumb_x, (int)track_y, thumb_r - 1, accent);

    // Value label centred at top of the bounds
    const char *label = TextFormat("%.1f", s->value);
    int fs = 13;
    int tw = MeasureText(label, fs);
    DrawText(label, (int)(b.x + b.width * 0.5f) - tw / 2, (int)b.y + 3, fs, DARKGRAY);
}
