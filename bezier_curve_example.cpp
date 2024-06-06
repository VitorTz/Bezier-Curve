#include "bezier.h"
#include <iostream>


#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720
#define WINDOW_TITLE "Bezier"
#define PADDING 50.0f

#define CIRCLE_RADIUS 10.f

#define NUM_CONTROL_POINTS 2.0
#define STEP 1.0 / (NUM_CONTROL_POINTS + 1.0)


typedef struct mouse {
    bool dragged = false;
    int last_circle_dragged = -1;
} mouse_t;


void handle_mouse(bz::bezier_animation_t* animation, mouse_t* mouse) {
    const Vector2 mouse_pos = GetMousePosition();
    bool right_mouse = IsMouseButtonReleased(MOUSE_RIGHT_BUTTON);
    bool space = IsKeyPressed(KEY_SPACE);

    for (int i = 0; i < animation->control_points.size(); i++) {
        const bool circle_collide = CheckCollisionPointCircle(mouse_pos, animation->control_points[i], CIRCLE_RADIUS);
        const bool line_collide = CheckCollisionPointLine(
            mouse_pos,
            animation->control_points[i],
            animation->control_points[i+1],
            8
        );
        if (space && circle_collide) {
            bz::remove_point(animation, i);
        } else if (circle_collide) {
            mouse->last_circle_dragged = i;
        }        
        if (right_mouse && line_collide)  {            
            right_mouse = false;
            bz::insert_control_point(animation, mouse_pos, i);            
        }
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


void draw_animation(bz::bezier_animation_t* animation) {
    const int n = animation->control_points.size();
    for (int i = 0; i < n; i++) {
        DrawCircleV(animation->control_points[i], CIRCLE_RADIUS, i == 0 || i == n - 1 ? RED : BROWN);
    }
    for (int i = 0; i < n - 1; i++) {
        DrawLineV(animation->control_points[i], animation->control_points[i+1], BROWN);
    }
    DrawCircleV(animation->C, CIRCLE_RADIUS, BLUE);
}


int main(int argc, char const *argv[]) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);    
    mouse_t* mouse = new mouse_t{};
    bz::bezier_animation_t* animation = bz::bezier_animation_create(
        5.0,
        {PADDING, PADDING},
        {SCREEN_WIDTH - PADDING, SCREEN_HEIGHT - PADDING},
        true,
        bz::TBasicFunction::Parabola
    );
    for (int i = 0; i < NUM_CONTROL_POINTS; i++) {
        bz::add_control_point_lerp(animation, STEP*(i+1));
    }    
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        BeginDrawing();
        ClearBackground(GetColor(0x181818ff));
            handle_mouse(animation, mouse);
            bz::animation_update(animation, dt);
            draw_animation(animation);
        EndDrawing();
    }
    
    bezier_animation_destroy(animation);    
    delete mouse;
    CloseWindow();
    return 0;
}
