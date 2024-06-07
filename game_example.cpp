#include "bezier.h"
#include <random>
#include <array>
#include <vector>
#include <iostream>
#include <queue>


#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720
#define SCREEN_RECT (Rectangle{0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT})
#define WINDOW_TITLE "Bezier"
#define WINDOW_COLOR (GetColor(0x181818ff))

#define BULLET_RADIUS 4.0f
#define PLAYER_NORMAL_BULLET_COLOR (YELLOW)
#define PLAYER_SPECIAL_BULLET_COLOR (WHITE)

#define PLAYER_COLOR (BLUE)
#define PLAYER_RADIUS 10.0f
#define PLAYER_SPEED 200.0f
#define PLAYER_SLOW_SPEED 120.0f
#define PLAYER_ATTACK_SPEED 0.2f

#define ENEMY_ATTACK_SPEED 0.5f
#define ENEMY_COLOR (RED)
#define ENEMY_RADIUS 5.f
#define ENEMY_BULLET_COLOR (BROWN)

#define MAIN_ENEMY_COLOR (BLUE)
#define MAIN_ENEMY_RADIUS 10.f
#define MAIN_ENEMY_OFFSET 50.f


std::default_random_engine generator;
std::uniform_int_distribution<int> randEnemyNum(1, 5);
std::uniform_real_distribution<float> randOffset(-20.f, 20.f);
std::uniform_real_distribution<float> randXPos(0.f, (float) SCREEN_WIDTH);
std::uniform_real_distribution<float> randYPos(0.f, (float) SCREEN_HEIGHT);


std::vector<bz::bezier_animation_t> enemy_bullets;
std::vector<bz::bezier_animation_t> special_bullets;
std::vector<bz::bezier_animation_t> normal_bullets;
bz::bezier_animation_t enemy_animation;

Vector2 player_pos;

double player_timer = 0.0;
double enemy_timer = 0.0;


void create_enemy_bullets() {
    if (enemy_timer >= ENEMY_ATTACK_SPEED) {
        enemy_timer = 0.0;
        const int n = randEnemyNum(generator);
        for (int i = 0; i < n; i++) {
            bz::bezier_animation_t animation = {};
            animation.time_to_complete = 8.f;            
            animation.control_points.push_back(enemy_animation.C);
            animation.control_points.push_back({randXPos(generator), randYPos(generator)});
            animation.control_points.push_back({player_pos.x, player_pos.y + SCREEN_HEIGHT});
            enemy_bullets.push_back(animation);
        }
    }
}


void create_player_bullets() {
    if (player_timer >= PLAYER_ATTACK_SPEED) {
        player_timer = 0.0;
        std::array<int, 2> directions = {-1, 1};
        bz::bezier_animation_t animation;
        animation.time_to_complete = 2.0f;
        for (int d : directions) {
            // normal bullet
            animation.control_points.clear();
            animation.control_points.push_back(
                {player_pos.x + 20 * d, player_pos.y - 20}
            );
            animation.control_points.push_back(
                {player_pos.x + 20 * d, -100}
            );            
            normal_bullets.push_back(animation);
            // special bullet
            animation.control_points.clear();
            animation.control_points.push_back({player_pos.x + 100 * d, player_pos.y - 20});
            animation.control_points.push_back({player_pos.x + 500 * d, player_pos.y - 200});
            animation.control_points.push_back({0.f, 0.f});  
            special_bullets.push_back(animation);
        }                
    }
}


void update_bullets(
    std::vector<bz::bezier_animation_t>* bullets,
    const float dt,
    Vector2* target
) {
   for (bz::bezier_animation_t& a : *bullets) {
        bz::bezier_animation_t* pA = std::addressof(a);
        if (target != NULL) { 
            bz::change_end_point(pA, *target);
        }
        bz::animation_update(pA, dt);
   } 
}


void handle_offscreen_bullets(std::vector<bz::bezier_animation_t>* bullets) {
    std::queue<int> q;
    for (int i = 0; i < bullets->size(); i++) {
        bz::bezier_animation_t& a = bullets->at(i);
        if (a.time_count >= a.time_to_complete) {
            q.push(i);
        }
    }
    while (q.empty() == false) {        
        bullets->erase(bullets->begin() + q.front());
        q.pop();
    }
}

void update_player(const float dt) {
    const float speed = IsKeyDown(KEY_LEFT_SHIFT) ? PLAYER_SLOW_SPEED * dt : PLAYER_SPEED * dt;
    Vector2 direction = {0.f, 0.f};
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
    player_pos.x += direction.x * speed;
    player_pos.y += direction.y * speed;
}

void update(const float dt) {
    bz::animation_update(&enemy_animation, dt);
    update_player(dt);
    create_enemy_bullets();
    create_player_bullets();
    update_bullets(&normal_bullets, dt, NULL);
    update_bullets(&enemy_bullets, dt, NULL);
    update_bullets(&special_bullets, dt, &enemy_animation.C);
    handle_offscreen_bullets(&normal_bullets);
    handle_offscreen_bullets(&enemy_bullets);
    handle_offscreen_bullets(&special_bullets);
}


void draw() {
    for (bz::bezier_animation_t& a : normal_bullets) {
        DrawCircleV(a.C, BULLET_RADIUS, PLAYER_NORMAL_BULLET_COLOR);
    }
    for (bz::bezier_animation_t& a : special_bullets) {
        DrawCircleV(a.C, BULLET_RADIUS, PLAYER_SPECIAL_BULLET_COLOR);
    }
    for (bz::bezier_animation_t& a : enemy_bullets) {
        DrawCircleV(a.C, BULLET_RADIUS, ENEMY_BULLET_COLOR);
    }
    DrawCircleV(enemy_animation.C, ENEMY_RADIUS, ENEMY_COLOR);
    DrawCircleV(player_pos, PLAYER_RADIUS, PLAYER_COLOR);
}




int main(int argc, char const *argv[]) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);        
    enemy_animation.control_points.push_back({MAIN_ENEMY_OFFSET, MAIN_ENEMY_OFFSET});
    enemy_animation.control_points.push_back({SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f});
    enemy_animation.control_points.push_back({SCREEN_WIDTH - MAIN_ENEMY_OFFSET, MAIN_ENEMY_OFFSET});
    enemy_animation.time_to_complete = 9.0;
    enemy_animation.loop = true;
    enemy_animation.t_function = bz::TBasicFunction::Parabola;
    
    player_pos = {
        SCREEN_WIDTH / 2.0 - PLAYER_RADIUS / 2.0, 
        SCREEN_HEIGHT - 50.0
    };    

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
            player_timer += dt;
            enemy_timer += dt;    
            update(dt);
        BeginDrawing();
        ClearBackground(WINDOW_COLOR);
            draw();
        EndDrawing();
    }

    CloseWindow();

}
