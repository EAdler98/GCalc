#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>
#include "token.h"

#define TEXTBOX_MAX 256

enum { TB_X = 10, TB_W = 320, TB_H = 36, TB_MARGIN = 10, TB_PAD = 4 };

typedef struct
{
    char *text;
    int len;
    int cursor; // insertion point, 0..len
    bool active;
    int font_size; // set to 0 to auto-fit the bounds height
    Font font;     // leave as (Font){0} to use the raylib default font
} Textbox;

static const Color palette[] = {RED, BLUE, ORANGE, PURPLE, MAROON, DARKBLUE, PINK};

typedef struct
{
    float value;
    bool dragging;
} Slider;
typedef struct
{
    Token *tokens;
    Color color;
    Textbox tb;
    float thickness;
} Function;

typedef struct
{
    int to_remove;
    int to_reparse;
    bool do_add;
    bool any_textbox_active;
} FunctionPanelResult;

typedef enum {
    INTERSECT_ALL  = 0,
    INTERSECT_NONE = 1
} IntersectMode;

// Draws and updates a raygui textbox, returns true when the text content changes.
bool textbox_update(Textbox *tb, Rectangle bounds);
void textbox_draw(const Textbox *tb, Rectangle bounds);

// Returns true when value changes
bool slider_update(Slider *s, Rectangle bounds);
void slider_draw(const Slider *s, Rectangle bounds, Color accent);
FunctionPanelResult draw_functions_tbs(Function *f, int count, int padding);
void ui_set_dark_mode(bool enabled);
// Immediate-mode button: draws and returns true when clicked this frame
bool button(Rectangle b, const char *label);
// Toggle group: updates *active (0-indexed); returns true when selection changes
bool toggle_group(Rectangle b, const char *labels, int *active);

#endif
