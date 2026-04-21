#include "storage.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *CONFIG_DIR_CANDIDATES[] = {
    "../config",
    "config",
    "../../config"
};
static const char *CONFIG_FILENAME = "gcalc_user_config.txt";
static const char *FUNCTIONS_FILENAME = "gcalc_functions.txt";
static const char *LEGACY_CONFIG_PATH = "gcalc_user_config.txt";
static const char *LEGACY_FUNCTIONS_PATH = "gcalc_functions.txt";

static void trim_line_end(char *text)
{
    if (text == NULL) return;
    text[strcspn(text, "\r\n")] = '\0';
}

static void clear_function(Function *function)
{
    if (function == NULL) return;
    free(function->tokens);
    free(function->tb.text);
    *function = (Function){0};
}

static char *alloc_text_buffer(const char *text)
{
    char *buffer = calloc(TEXTBOX_MAX, sizeof(char));
    if (buffer == NULL) return NULL;

    if (text != NULL) {
        strncpy(buffer, text, TEXTBOX_MAX - 1);
        buffer[TEXTBOX_MAX - 1] = '\0';
    }

    return buffer;
}

static bool resolve_config_dir(char *out_path, size_t out_size, bool create_if_missing)
{
    const char *app_dir = GetApplicationDirectory();
    char candidate_path[512];

    for (int i = 0; i < (int)(sizeof(CONFIG_DIR_CANDIDATES) / sizeof(CONFIG_DIR_CANDIDATES[0])); i++) {
        const char *candidate = CONFIG_DIR_CANDIDATES[i];
        if (DirectoryExists(candidate)) {
            snprintf(out_path, out_size, "%s", candidate);
            return true;
        }

        snprintf(candidate_path, sizeof(candidate_path), "%s/%s", app_dir, candidate);
        if (DirectoryExists(candidate_path)) {
            snprintf(out_path, out_size, "%s", candidate_path);
            return true;
        }
    }

    snprintf(out_path, out_size, "config");
    if (create_if_missing) {
        MakeDirectory(out_path);
        return true;
    }

    return false;
}

static void build_config_path(char *out_path, size_t out_size, const char *filename, bool create_if_missing)
{
    char config_dir[512];
    resolve_config_dir(config_dir, sizeof(config_dir), create_if_missing);
    snprintf(out_path, out_size, "%s/%s", config_dir, filename);
}

static FILE *open_storage_file(const char *filename, const char *legacy_path, const char *mode)
{
    bool create_if_missing = strchr(mode, 'w') != NULL || strchr(mode, 'a') != NULL;
    char path[512];
    build_config_path(path, sizeof(path), filename, create_if_missing);

    FILE *file = fopen(path, mode);
    if (file == NULL && !create_if_missing && legacy_path != NULL) {
        file = fopen(legacy_path, mode);
    }

    return file;
}

static int load_fallback_functions(Function **out_functions, float thickness)
{
    *out_functions = NULL;
    int default_count = 0;
    if (!append_function(out_functions, &default_count, "1/x", thickness, false)) return 0;
    if (!append_function(out_functions, &default_count, "-x^2 +3", thickness, false)) {
        free_functions(*out_functions, default_count);
        *out_functions = NULL;
        return 0;
    }

    return default_count;
}

UserConfig default_user_config(void)
{
    return (UserConfig){
        .dark_mode = true,
        .thickness = 3.0f,
        .intersect_mode = INTERSECT_ALL
    };
}

Function make_function_from_text(const char *text, int index, float thickness, bool active)
{
    Function function = {0};
    char *buffer = alloc_text_buffer(text != NULL ? text : "");
    if (buffer == NULL) return function;

    function.tokens = parse(buffer);
    function.color = palette[index % (sizeof(palette) / sizeof(palette[0]))];
    function.tb.text = buffer;
    function.tb.len = (int)strlen(buffer);
    function.tb.cursor = function.tb.len;
    function.tb.font_size = 22;
    function.tb.active = active;
    function.thickness = thickness;
    return function;
}

bool append_function(Function **functions, int *count, const char *text, float thickness, bool active)
{
    if (functions == NULL || count == NULL) return false;

    Function function = make_function_from_text(text, *count, thickness, active);
    if (function.tb.text == NULL) return false;

    Function *resized = realloc(*functions, (*count + 1) * sizeof(Function));
    if (resized == NULL) {
        clear_function(&function);
        return false;
    }

    *functions = resized;
    (*functions)[*count] = function;
    (*count)++;
    return true;
}

void remove_function_at(Function *functions, int *count, int index)
{
    if (functions == NULL || count == NULL || index < 0 || index >= *count) return;

    clear_function(&functions[index]);
    memmove(&functions[index], &functions[index + 1], (*count - index - 1) * sizeof(Function));
    (*count)--;
}

void set_all_function_thickness(Function *functions, int count, float thickness)
{
    for (int i = 0; i < count; i++) {
        functions[i].thickness = thickness;
    }
}

void free_functions(Function *functions, int count)
{
    if (functions == NULL) return;

    for (int i = 0; i < count; i++) {
        clear_function(&functions[i]);
    }

    free(functions);
}

UserConfig load_user_config(void)
{
    UserConfig config = default_user_config();
    FILE *file = open_storage_file(CONFIG_FILENAME, LEGACY_CONFIG_PATH, "r");
    if (file == NULL) return config;

    char line[128];
    while (fgets(line, sizeof(line), file) != NULL) {
        int int_value = 0;
        float float_value = 0.0f;

        if (sscanf(line, "dark_mode=%d", &int_value) == 1) {
            config.dark_mode = int_value != 0;
        } else if (sscanf(line, "thickness=%f", &float_value) == 1) {
            config.thickness = float_value;
        } else if (sscanf(line, "intersect_mode=%d", &int_value) == 1) {
            config.intersect_mode = (IntersectMode)int_value;
        }
    }

    fclose(file);

    if (config.thickness < 1.0f) config.thickness = 1.0f;
    if (config.thickness > 5.0f) config.thickness = 5.0f;
    if (config.intersect_mode < INTERSECT_ALL || config.intersect_mode > INTERSECT_NONE) {
        config.intersect_mode = INTERSECT_ALL;
    }

    return config;
}

void save_user_config(UserConfig config)
{
    FILE *file = open_storage_file(CONFIG_FILENAME, NULL, "w");
    if (file == NULL) return;

    fprintf(file, "dark_mode=%d\n", config.dark_mode ? 1 : 0);
    fprintf(file, "thickness=%.2f\n", config.thickness);
    fprintf(file, "intersect_mode=%d\n", (int)config.intersect_mode);
    fclose(file);
}

int load_saved_functions(Function **out_functions, float thickness)
{
    if (out_functions == NULL) return 0;

    FILE *file = open_storage_file(FUNCTIONS_FILENAME, LEGACY_FUNCTIONS_PATH, "r");
    if (file == NULL) return load_fallback_functions(out_functions, thickness);

    char line[TEXTBOX_MAX + 32];
    int count = 0;
    if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "count=%d", &count) != 1 || count < 0) {
        fclose(file);
        return load_fallback_functions(out_functions, thickness);
    }

    Function *functions = NULL;
    int loaded_count = 0;
    for (int i = 0; i < count; i++) {
        if (fgets(line, sizeof(line), file) == NULL) line[0] = '\0';
        trim_line_end(line);
        if (!append_function(&functions, &loaded_count, line, thickness, false)) {
            free_functions(functions, loaded_count);
            fclose(file);
            return load_fallback_functions(out_functions, thickness);
        }
    }

    fclose(file);
    *out_functions = functions;
    return loaded_count;
}

void save_functions_to_file(Function *functions, int count)
{
    FILE *file = open_storage_file(FUNCTIONS_FILENAME, NULL, "w");
    if (file == NULL) return;

    fprintf(file, "count=%d\n", count);
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s\n", functions[i].tb.text != NULL ? functions[i].tb.text : "");
    }

    fclose(file);
}
