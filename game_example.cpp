#include "bezier.h"
#include <vector>
#include <iostream>
#include <queue>


#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720
#define SCREEN_RECT (Rectangle{0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT})
#define WINDOW_TITLE "Bezier"
#define WINDOW_COLOR (GetColor(0x181818ff))

#define BULLET_RADIUS 4.0
#define BULLET_COLOR (YELLOW)
#define MAX_BULLETS 1000

#define PLAYER_COLOR (RED)
#define PLAYER_RADIUS 10.0
#define PLAYER_SPEED 250.0
#define PLAYER_SLOW_SPEED 80.0
#define PLAYER_ATTACK_SPEED 0.2


std::vector<bezier_animation_t*> special_bullets{};
std::vector<bezier_animation_t*> bullets{};

Vector2 player_pos = {SCREEN_WIDTH / 2.f, SCREEN_HEIGHT - 50.f};
Vector2 enemy_pos = Vector2Zero();

double time_count = 0.0;


void create_normal_bullet(int direction) {
    bezier_animation_t* animation = bezier_animation_create(
        1.0,
        {player_pos.x + 10 * direction, player_pos.y - 20},
        {player_pos.x + 10 * direction, -10},
        false,
        LerpFT::Quadratic
    );    
    bullets.push_back(animation);
}


void create_special_bullet(int direction) {    
    bezier_animation_t* animation = bezier_animation_create(
        2.0,
        {player_pos.x + 30 * direction, player_pos.y - 5},
        enemy_pos,
        false,
        LerpFT::SquareRoot
    );
    bezier_animation_add_control_point(
        animation, 
        {player_pos.x + 300 * direction, player_pos.y - 40}
    );
    special_bullets.push_back(animation);
}



void update_bullets_vec(std::vector<bezier_animation_t*>* vec, const float dt) {
    std::queue<int> offscreen_bullets;
    for (int i = 0; i < vec->size(); i++) {
        bezier_animation_t* a = vec->at(i);        
        bezier_animation_update(a, dt);
        if (!CheckCollisionCircleRec(a->C, BULLET_RADIUS, SCREEN_RECT)) {
            offscreen_bullets.push(i);
        }
    }
    while (offscreen_bullets.empty() == false) {
        const int b = offscreen_bullets.front();
        offscreen_bullets.pop();
        bezier_animation_destroy(vec->at(b));
        vec->erase(vec->begin() + b);
    }
}


void update_bullets_target(std::vector<bezier_animation_t*>* vec, const Vector2 target) {
    for (bezier_animation_t* a : *vec) {
        bezier_animation_change_target(a, target);
    }
}


void add_bullets() {
    if (time_count >= PLAYER_ATTACK_SPEED && bullets.size() < MAX_BULLETS) {
        time_count = 0.0;
        create_special_bullet(1);
        create_special_bullet(-1);
        create_normal_bullet(1);
        create_normal_bullet(-1);
    }
}


void update_bullets(const float dt) {        
    add_bullets();
    update_bullets_target(&special_bullets, enemy_pos);
    update_bullets_vec(&special_bullets, dt);
    update_bullets_vec(&bullets, dt);    
}


void update_player(const float dt) {
    Vector2 direction = Vector2Zero();
    if (IsKeyDown(KEY_LEFT)) {
        direction.x = -1;
    } else if (IsKeyDown(KEY_RIGHT)) {
        direction.x = 1;
    }
    if (IsKeyDown(KEY_UP)) {
        direction.y = -1;
    } else if (IsKeyDown(KEY_DOWN)) {
        direction.y = 1;
    }
    direction = Vector2Normalize(direction);
    const float speed = IsKeyDown(KEY_LEFT_SHIFT) ? PLAYER_SLOW_SPEED : PLAYER_SPEED;
    const float distance = speed * dt;
    player_pos.x += direction.x * distance;
    player_pos.y += direction.y * distance;
}


void draw_enemy() {
    DrawCircleV(enemy_pos, 10.f, BLUE);
}


void draw_player() {
    DrawCircleV(player_pos, PLAYER_RADIUS,  PLAYER_COLOR);
}


void draw_bullets() {
    for (bezier_animation_t* a : special_bullets) {
        DrawCircleV(a->C, BULLET_RADIUS, BULLET_COLOR);
    }
    for (bezier_animation_t* a : bullets) {
        DrawCircleV(a->C, BULLET_RADIUS, BULLET_COLOR);
    }
}


void destroy_animations(const std::vector<bezier_animation_t*>& animation_vec) {
    for (bezier_animation_t* a : animation_vec) {
        bezier_animation_destroy(a);
    }
}


int main(int argc, char const *argv[]) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);    
        
    bezier_animation_t* enemy_animation = bezier_animation_create(
        8.0,
        {50.f, 50.f},
        {SCREEN_WIDTH - 50, 50.f},
        true,
        LerpFT::Parabola
    );
    bezier_animation_add_control_point(enemy_animation, {500.f, 200.f});    

    while (!WindowShouldClose()) {

        const float dt = GetFrameTime();
        time_count += dt;
            bezier_animation_update(enemy_animation, dt);
            enemy_pos = enemy_animation->C;
            update_bullets(dt);
            update_player(dt);

        BeginDrawing();
        ClearBackground(WINDOW_COLOR);
            draw_enemy();                                 
            draw_bullets();            
            draw_player();
        EndDrawing();                                                                                                  

    }

    destroy_animations(bullets);
    destroy_animations(special_bullets);    
    CloseWindow();
    return 0;
}
