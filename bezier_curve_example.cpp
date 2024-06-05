#include "bezier.h"
#include <iostream>


#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720
#define WINDOW_TITLE "Bezier"
#define PADDING 50.0f

#define CIRCLE_RADIUS 10.f

#define NUM_CONTROL_POINTS 2.0
#define STEP 1.0 / (NUM_CONTROL_POINTS + 1.0)

void print_vec(Vector2 v) {
    printf("Vec(%.2f, %.2f)\n", v.x, v.y);
}


typedef struct mouse {
    bool dragged = false;
    int last_circle_dragged = -1;
} mouse_t;


void handle_mouse(bezier_animation_t* animation, mouse_t* mouse) {
    const Vector2 mouse_pos = GetMousePosition();
    const bool right_mouse = IsMouseButtonReleased(MOUSE_RIGHT_BUTTON);
    
    int pos_to_insert_point = -1;
    for (int i = 0; i < animation->control_points.size(); i++) {
        if (CheckCollisionPointCircle(mouse_pos, animation->control_points[i], CIRCLE_RADIUS)) {
            mouse->last_circle_dragged = i;
        }
        if (
            right_mouse &&
            CheckCollisionPointLine(
                mouse_pos,
                animation->control_points[i],
                animation->control_points[i+1],
                8
            )
        )  {
            pos_to_insert_point = i + 1;
        }
    }
    
    if (pos_to_insert_point != -1) {
        animation->control_points.insert(
            animation->control_points.begin() + pos_to_insert_point, 
            mouse_pos
        );        
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        mouse->dragged = false;
        mouse->last_circle_dragged = -1;
        return;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        mouse->dragged = true;
        mouse->last_circle_dragged = -1;
    }
    
    if (mouse->dragged && mouse->last_circle_dragged != -1) {
        animation->control_points[mouse->last_circle_dragged] = mouse_pos;
    }

}


void update_animation(bezier_animation_t* animation, const float dt) {
    if (IsKeyPressed(KEY_SPACE)) {
        animation->progress.time_count = 0.0;
        animation->progress.reverse = false;
    }
    bezier_animation_update(animation, dt);
}


void draw_circles(bezier_animation_t* animation) {
    const int n = animation->control_points.size();
    for (int i = 0; i < n; i++) {
        DrawCircleV(animation->control_points[i], CIRCLE_RADIUS, i == 0 || i == n - 1 ? RED : BROWN);
    }
}


void draw_lines(bezier_animation_t* animation) {
    for (int i = 0; i < animation->control_points.size() - 1; i++) {
        DrawLineV(animation->control_points[i], animation->control_points[i+1], BROWN);
    }
}


bezier_animation_t* init_bezier() {
    bezier_animation_t* animation = bezier_animation_create(
        5.0,
        {PADDING, PADDING},
        {SCREEN_WIDTH - PADDING, SCREEN_HEIGHT - PADDING},
        true,
        LerpFT::Parabola
    );
    for (int i = 0; i < NUM_CONTROL_POINTS; i++) {
        bezier_animation_add_control_point(animation, STEP*(i+1));
    }    
    return animation;
}


int main(int argc, char const *argv[]) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);    
    bezier_animation_t* animation = init_bezier();
    mouse_t* mouse = new mouse_t{};

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818ff));
            handle_mouse(animation, mouse);
            update_animation(animation, GetFrameTime());

            draw_lines(animation);
            draw_circles(animation);
            DrawCircleV(animation->C, CIRCLE_RADIUS, BLUE);
        EndDrawing();
    }
    
    bezier_animation_destroy(animation);
    free(mouse);
    CloseWindow();
    return 0;
}
