#include "ui.h"

bool textbox_update(Textbox *tb, Rectangle bounds)
{
    int prev_len=tb->len;
    // click to focus / unfocus
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        tb->active = CheckCollisionPointRec(GetMousePosition(), bounds);

    if (!tb->active) return false;

    // character input
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && tb->len < TEXTBOX_MAX - 1) {
            tb->text[tb->len++] = (char)key;
            tb->text[tb->len]   = '\0';
        }
        key = GetCharPressed();
    }

    // backspace
    if (IsKeyPressed(KEY_BACKSPACE) && tb->len > 0)
        tb->text[--tb->len] = '\0';

    if (tb->len!=prev_len) {
        return true;
    }

    return false;
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

    // blinking cursor
    if (tb->active && ((int)(GetTime() * 2) % 2)) {
        Vector2 measured = MeasureTextEx(font, tb->text, fs, 1);
        int cx = (int)b.x + 6 + (int)measured.x;
        DrawLine(cx, (int)b.y + 4, cx, (int)b.y + (int)b.height - 4, DARKGRAY);
    }
}
