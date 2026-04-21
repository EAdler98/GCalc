#ifndef STORAGE_H
#define STORAGE_H

#include "ui.h"

typedef struct
{
    bool dark_mode;
    float thickness;
    IntersectMode intersect_mode;
} UserConfig;

UserConfig default_user_config(void);
UserConfig load_user_config(void);
void save_user_config(UserConfig config);

Function make_function_from_text(const char *text, int index, float thickness, bool active);
bool append_function(Function **functions, int *count, const char *text, float thickness, bool active);
void remove_function_at(Function *functions, int *count, int index);
void set_all_function_thickness(Function *functions, int count, float thickness);
void free_functions(Function *functions, int count);

int load_saved_functions(Function **out_functions, float thickness);
void save_functions_to_file(Function *functions, int count);

#endif
