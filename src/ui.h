#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>

#define TEXTBOX_MAX 256

typedef struct {
    char * text;
    int  len;
    bool active;
    int  font_size; // set to 0 to auto-fit the bounds height
    Font font;      // leave as (Font){0} to use the raylib default font
} 
Textbox;

// Returns true when the user submits (Enter key)
bool textbox_update(Textbox *tb, Rectangle bounds);
void textbox_draw(const Textbox *tb, Rectangle bounds);

#endif
