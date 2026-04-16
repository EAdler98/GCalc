#include "ui.h"
#include <string.h>
#include <math.h>

#define RAYGUI_IMPLEMENTATION
#include "../build/external/raylib-master/examples/core/raygui.h"


const float min = 1.0f;
const float max = 5.0f;
const float step =0.5f;

static void ensure_raygui_style(void)
{
    static bool initialized = false;
    if (initialized) return;

    GuiLoadStyleDefault();
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(TEXTBOX, TEXT_PADDING, 8);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(SLIDER, TEXT_PADDING, 6);

    initialized = true;
}

bool textbox_update(Textbox *tb, Rectangle bounds)
{
    if ((tb == NULL) || (tb->text == NULL)) return false;

    ensure_raygui_style();

    char previous[TEXTBOX_MAX];
    strncpy(previous, tb->text, TEXTBOX_MAX - 1);
    previous[TEXTBOX_MAX - 1] = '\0';

    if (GuiTextBox(bounds, tb->text, TEXTBOX_MAX, tb->active)) tb->active = !tb->active;

    tb->len = (int)strlen(tb->text);
    tb->cursor = tb->len;

    return strcmp(previous, tb->text) != 0;
}

void textbox_draw(const Textbox *tb, Rectangle b)
{
    if ((tb == NULL) || (tb->text == NULL)) return;

    ensure_raygui_style();
    GuiTextBox(b, tb->text, TEXTBOX_MAX, false);
}

bool slider_update(Slider *s, Rectangle b)
{
    if (s == NULL) return false;

    ensure_raygui_style();

    float prev = s->value;
    GuiSliderBar(b, NULL, NULL, &s->value, min, max);
    s->value = roundf(s->value / step) * step;
    s->value = s->value < min ? min : s->value > max ? max : s->value;
    s->dragging = IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), b);

    return s->value != prev;
}

void slider_draw(const Slider *s, Rectangle b, Color accent)
{
    (void)accent;
    if ((s == NULL)) return;

    ensure_raygui_style();
    GuiSliderBar(b, NULL, TextFormat("%.1f", s->value), (float *)&s->value, min, max);
}

bool button(Rectangle b, const char *label)
{
    ensure_raygui_style();
    return GuiButton(b, label);
}

FunctionPanelResult draw_functions_tbs(Function *f, int count, int padding)
{
    FunctionPanelResult result = { .to_remove = -1, .to_reparse = -1, .do_add = false, .any_textbox_active = false };

    Rectangle tb_s = { TB_X, GetScreenHeight() - TB_MARGIN - TB_H, TB_W, TB_H };
    for (int i = 0; i < count; i++) {
        Rectangle b  = { tb_s.x, tb_s.y - i * (TB_H + padding), tb_s.width, tb_s.height };
        Rectangle cb = { b.x - 18, b.y + 8, 10, 10 };
        Rectangle sb = { b.x + b.width + 8, b.y, 140, b.height };
        Rectangle mb = { sb.x + sb.width + 8, b.y, b.height, b.height };

        DrawRectangleRec(cb, f[i].color);
        if (textbox_update(&f[i].tb, b)) result.to_reparse = i;
        result.any_textbox_active = result.any_textbox_active || f[i].tb.active;

        if (slider_update(&f[i].slider, sb)) f[i].thickness = f[i].slider.value;
        if (button(mb, "-")) result.to_remove = i;
    }

    Rectangle plus_b = { TB_X, tb_s.y - count * (TB_H + padding), TB_H, TB_H };
    if (button(plus_b, "+")) result.do_add = true;

    return result;
}
