#include "ui.h"
#include <string.h>

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
