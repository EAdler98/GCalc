#include "raylib.h"
#include "parser.h"
#include "graph.h"
#include "storage.h"
#include <stdlib.h>

static const int SCREEN_WIDTH = 1920;
static const int SCREEN_HEIGHT = 1080;
static const int STARTUP_FPS = 120;
static const float SCALE = 20.0f;
static const Color DARK_BACKGROUND = { 13, 17, 23, 255 };
static const Color LIGHT_BACKGROUND = RAYWHITE;
static const Color DARK_HUD_TEXT = { 230, 237, 243, 255 };
static const Color LIGHT_HUD_TEXT = DARKGRAY;

typedef struct
{
    Function *functions;
    int function_count;
    Camera2D camera;
    Vector2 target_pos;
    float target_zoom;
    float idle_timer;
    int current_fps;
    bool dark_mode;
    Slider thickness_slider;
    IntersectMode intersect_mode;
} AppState;

static Camera2D create_camera(void)
{
    Camera2D camera = {0};
    camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.zoom = 1.0f;
    return camera;
}

static UserConfig get_current_user_config(const AppState *state)
{
    return (UserConfig){
        .dark_mode = state->dark_mode,
        .thickness = state->thickness_slider.value,
        .intersect_mode = state->intersect_mode
    };
}

static void apply_theme(bool dark_mode)
{
    ui_set_dark_mode(dark_mode);
    set_graph_dark_mode(dark_mode);
}

static AppState create_app_state(void)
{
    UserConfig config = load_user_config();
    AppState state = {0};

    state.function_count = load_saved_functions(&state.functions, config.thickness);
    state.camera = create_camera();
    state.target_pos = state.camera.target;
    state.target_zoom = state.camera.zoom;
    state.current_fps = STARTUP_FPS;
    state.dark_mode = config.dark_mode;
    state.thickness_slider = (Slider){ config.thickness, false };
    state.intersect_mode = config.intersect_mode;
    return state;
}

static bool has_active_textboxes(const AppState *state)
{
    for (int i = 0; i < state->function_count; i++) {
        if (state->functions[i].tb.active) return true;
    }

    return false;
}

static void update_idle_frame_rate(AppState *state, bool any_textbox_active)
{
    float wheel = GetMouseWheelMove();
    bool is_user_active = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
                          IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ||
                          IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
                          wheel != 0 ||
                          any_textbox_active ||
                          IsKeyPressed(KEY_F11);

    if (is_user_active) {
        state->idle_timer = 0.0f;
        if (state->current_fps != 60) {
            state->current_fps = 60;
            SetTargetFPS(state->current_fps);
        }
    } else {
        state->idle_timer += GetFrameTime();
        if (state->idle_timer > 1.0f && state->current_fps != 20) {
            state->current_fps = 20;
            SetTargetFPS(state->current_fps);
        }
    }
}

static void draw_graph_scene(const AppState *state)
{
    ClearBackground(state->dark_mode ? DARK_BACKGROUND : LIGHT_BACKGROUND);
    BeginMode2D(state->camera);
    draw_grid(state->camera, SCALE);
    draw_axes(state->camera);
    for (int i = 0; i < state->function_count; i++) {
        draw_function(state->functions[i], state->camera, SCALE);
    }
    draw_intersections(state->functions, state->function_count, state->camera, SCALE, state->intersect_mode);
    EndMode2D();
}

static FunctionPanelResult draw_interface(AppState *state)
{
    FunctionPanelResult panel = draw_functions_tbs(state->functions, state->function_count, 8);
    Rectangle theme_button = { 10, 34, 130, 28 };

    if (button(theme_button, state->dark_mode ? "Light Mode" : "Dark Mode")) {
        state->dark_mode = !state->dark_mode;
        apply_theme(state->dark_mode);
    }

    DrawText("Thickness", 10, 68, 16, state->dark_mode ? DARK_HUD_TEXT : LIGHT_HUD_TEXT);
    if (slider_update(&state->thickness_slider, (Rectangle){ 10, 88, 130, 24 })) {
        set_all_function_thickness(state->functions, state->function_count, state->thickness_slider.value);
    }

    DrawText("Crossings", 10, 118, 16, state->dark_mode ? DARK_HUD_TEXT : LIGHT_HUD_TEXT);
    {
        int current_mode = (int)state->intersect_mode;
        toggle_group((Rectangle){ 10, 138, 120, 24 }, "All;Hover;Off", &current_mode);
        state->intersect_mode = (IntersectMode)current_mode;
    }

    DrawText(TextFormat("Zoom: %.2f", state->camera.zoom), 10, 10, 18,
             state->dark_mode ? DARK_HUD_TEXT : LIGHT_HUD_TEXT);
    return panel;
}

static void reparse_function(Function *function)
{
    free(function->tokens);
    function->tokens = parse(function->tb.text);
}

static void handle_panel_result(AppState *state, FunctionPanelResult panel)
{
    if (panel.to_reparse >= 0) {
        reparse_function(&state->functions[panel.to_reparse]);
    }

    if (panel.to_remove >= 0) {
        remove_function_at(state->functions, &state->function_count, panel.to_remove);
    }

    if (panel.do_add) {
        append_function(&state->functions, &state->function_count, "", state->thickness_slider.value, true);
    }
}

int main(void)
{
    AppState state = create_app_state();
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GCalc");
    SetWindowFocused();
    SetTargetFPS(state.current_fps);
    apply_theme(state.dark_mode);

    while (!WindowShouldClose())
    {
        bool any_textbox_active = has_active_textboxes(&state);
        update_idle_frame_rate(&state, any_textbox_active);
        update_camera_smooth(&state.camera, &state.target_pos, &state.target_zoom);

        BeginDrawing();
        draw_graph_scene(&state);
        FunctionPanelResult panel = draw_interface(&state);
        EndDrawing();

        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        handle_panel_result(&state, panel);
    }

    save_user_config(get_current_user_config(&state));
    save_functions_to_file(state.functions, state.function_count);
    free_functions(state.functions, state.function_count);
    CloseWindow();
    return 0;
}
