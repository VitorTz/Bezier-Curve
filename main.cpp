#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <queue>
#include "bezier.h"


#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720
#define WINDOW_TITLE "A"

#define CIRCLE_RADIUS 10.0f
#define SCREEN_RECT (Rectangle{0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT})


std::vector<bezier_animation_t*> animations;


void print_vector(const Vector2 v) {
    std::cout << "Vec(" << v.x << ", " << v.y << ")\n";
}


Vector2 rotate_origin(
    const Vector2 origin, 
    const Vector2 point, 
    const double angle
) { 
    const double rad = angle * (M_PI / 180.0);    
    Vector2 p = {point.x - origin.x, point.y - origin.y};    
    float x = p.x * std::cos(rad) - p.y * std::sin(rad);
    float y = p.x * std::sin(rad) + p.y * std::cos(rad);    
    p.x = x + origin.x;
    p.y = y + origin.y;
    return p;
}


void rotate_points(
    const Vector2 origin, 
    const Vector2 offset, 
    const std::size_t num_points,
    std::vector<Vector2>* points
) {
    points->clear();     
    const double step = 360.0 / num_points;
    for (double angle = 0.0; angle < 360.0; angle+=step) {        
        if (angle > 360.0) { angle = 360.0; }
        points->push_back(rotate_origin(origin, offset, angle));        
    }
}


void create_bullets(const Vector2 origin, const std::size_t num_bullets) {
    std::vector<Vector2> points;
    const Vector2 pos = {origin.x + 100.f, origin.y};
    rotate_points(origin, pos, num_bullets, &points);
    int i = 0;
    for (const Vector2 v : points) {
        bezier_animation_t* animation = bezier_animation_create(
            4,
            v,
            {v.x + i*20, SCREEN_HEIGHT + 100.f},
            false,
            TBasicFunction::Quadratic
        );
        bezier_animation_add_control_point(
            animation, 
            {v.x >= origin.x ? v.x + 300.f : v.x - 300.f, v.y}
        );
        animations.push_back(animation);
    }
}


void update_animations(const float dt) {
    std::queue<int> to_remove;
    int i = 0;    
    for (bezier_animation_t* animation : animations) {
        bezier_animation_update(animation, dt);
        if (!CheckCollisionCircleRec(animation->C, CIRCLE_RADIUS, SCREEN_RECT)) {
            to_remove.push(i);
        }
        i++;
    }
    while (to_remove.empty() == false) {
        const int a = to_remove.front();
        to_remove.pop();
        animations.erase(animations.begin() + a);
    }
}


void draw_animations() {
    for (bezier_animation_t* animation : animations) {
        DrawCircleV(animation->C, CIRCLE_RADIUS, GREEN);
    }
}


int main(int argc, char const *argv[]) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);    
    Vector2 origin = {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f};    
    create_bullets(origin, 10);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x181818ff));
            update_animations(GetFrameTime());
            DrawCircleV(origin, CIRCLE_RADIUS, RED);
            draw_animations();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
