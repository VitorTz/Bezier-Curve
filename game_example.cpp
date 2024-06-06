#include "bezier.h"
#include <random>
#include <vector>
#include <iostream>
#include <queue>


#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720
#define SCREEN_RECT (Rectangle{0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT})
#define WINDOW_TITLE "Bezier"
#define WINDOW_COLOR (GetColor(0x181818ff))

#define BULLET_RADIUS 4.0f
#define BULLET_COLOR (YELLOW)

#define PLAYER_COLOR (RED)
#define PLAYER_RADIUS 10.0f
#define PLAYER_SPEED 400.0f
#define PLAYER_ATTACK_SPEED 0.2f

#define ENEMY_RATE 0.5f
#define ENEMY_COLOR (BROWN)
#define ENEMY_RADIUS 5.f

#define MAIN_ENEMY_COLOR (BLUE)
#define MAIN_ENEMY_RADIUS 10.f
#define MAIN_ENEMY_OFFSET 50.f


typedef struct bullet {
    Vector2 center = Vector2Zero();    
    bz::bezier_animation_t movement;
} bullet_t;


typedef struct enemy {
    Vector2 center = Vector2Zero();
    bz::bezier_animation_t movement;
} enemy_t;

std::default_random_engine generator;

std::uniform_int_distribution<int> randEnemyNum(1, 10);
std::uniform_real_distribution<float> randOffset(-20.f, 20.f);
std::uniform_real_distribution<float> randXPos(0.f, (float) SCREEN_WIDTH);
std::uniform_real_distribution<float> randYPos(0.f, (float) SCREEN_HEIGHT);

std::vector<bullet_t> special_bullets{};
std::vector<bullet_t> bullets{};
std::vector<enemy_t> enemys{};

Vector2 player_pos{};
Vector2 main_enemy_pos{};

double bullet_timer = 0.0;
double enemy_timer = 0.0;


void create_normal_bullet(const int direction) {
    bz::bezier_animation_t animation = {};
    animation.progress.time_to_complete = 2.0;
    animation.t_function = bz::TBasicFunction::Normal;
    animation.progress.loop = false;
    animation.control_points.push_back({player_pos.x + 20 * direction, player_pos.y - 20});
    animation.control_points.push_back({player_pos.x + 20 * direction, -100});
    bullets.push_back({.movement = animation});
}


void create_special_bullet(const int direction) {
    bz::bezier_animation_t animation = {};
    animation.progress.time_to_complete = 2.f;
    animation.t_function = bz::TBasicFunction::Normal;
    animation.progress.loop = false;
    animation.control_points.push_back({player_pos.x + 100 * direction, player_pos.y - 20});
    animation.control_points.push_back({player_pos.x + 500 * direction, player_pos.y - 200});
    animation.control_points.push_back({0.f, 0.f});    
    special_bullets.push_back({.movement = animation});
}


void create_enemy(const Vector2 endPos) {        
    bz::bezier_animation_t animation = {};
    animation.progress.time_to_complete = 10.f;
    animation.t_function = bz::TBasicFunction::Quadratic; 
    animation.progress.loop = false;
    animation.control_points.push_back(main_enemy_pos);
    animation.control_points.push_back({randXPos(generator), randYPos(generator)});
    animation.control_points.push_back(player_pos);
    enemys.push_back({.movement = animation});
}


void update_bullets_and_enemys(const float dt) {
    if (bullet_timer >= PLAYER_ATTACK_SPEED) {
        bullet_timer = 0.0;        
        create_normal_bullet(1);
        create_normal_bullet(-1);
        create_special_bullet(1);
        create_special_bullet(-1);        
    }

    if (enemy_timer >= ENEMY_RATE) {
        enemy_timer = 0.0;
        const int num_enemys = randEnemyNum(generator);        
        const Vector2 endPos = {randXPos(generator), randYPos(generator) + SCREEN_HEIGHT};
        for (int i = 0; i < num_enemys; i++) {
            create_enemy(endPos);
        }
    }
    
    std::queue<int> to_remove{};
    int i;

    for (i = 0; i < bullets.size(); i++) {
        bullet_t& bullet = bullets[i];        
        bz::animation_update(&bullet.movement, dt);
        bullet.center = bullet.movement.C;
        if (!CheckCollisionCircleRec(bullet.center, BULLET_RADIUS, SCREEN_RECT)) {
            to_remove.push(i);
        }
    }

    while (to_remove.empty() == false) {
        i = to_remove.front();
        to_remove.pop();
        bullets.erase(bullets.begin() + i);
    }

    for (i = 0; i < special_bullets.size(); i++) {
        bullet_t& bullet = special_bullets[i];
        bullet.movement.control_points[bullet.movement.control_points.size()-1] = main_enemy_pos;
        bz::animation_update(&bullet.movement, dt);
        bullet.center = bullet.movement.C;
        if (!CheckCollisionCircleRec(bullet.center, BULLET_RADIUS, SCREEN_RECT)) {
            to_remove.push(i);
        }
    }

    while (to_remove.empty() == false) {
        i = to_remove.front();
        to_remove.pop();
        special_bullets.erase(special_bullets.begin() + i);
    }

    for (int i = 0; i < enemys.size(); i++) {
        enemy_t& enemy = enemys[i];        
        bz::animation_update(&enemy.movement, dt);
        enemy.center = enemy.movement.C;        
        if (enemy.center.y > SCREEN_HEIGHT) {
            to_remove.push(i);
        } 
    }
    
    while (to_remove.empty() == false) {
        i = to_remove.front();
        to_remove.pop();
        enemys.erase(enemys.begin() + i);
    }

}


void update_player(const float dt) {
    Vector2 direction = Vector2Zero();
    const float distance = PLAYER_SPEED * dt;
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
    player_pos.x += direction.x * distance;
    player_pos.y += direction.y * distance;
}



int main(int argc, char const *argv[]) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    bz::bezier_animation_t enemy_animation = {};
    enemy_animation.progress.time_to_complete = 3.0;
    enemy_animation.progress.loop = true;
    enemy_animation.t_function = bz::TBasicFunction::Parabola;
    enemy_animation.control_points.push_back({MAIN_ENEMY_OFFSET, MAIN_ENEMY_OFFSET});
    enemy_animation.control_points.push_back({SCREEN_WIDTH - MAIN_ENEMY_OFFSET, MAIN_ENEMY_OFFSET});

    player_pos.x = SCREEN_WIDTH / 2.0 - PLAYER_RADIUS / 2.0;
    player_pos.y = SCREEN_HEIGHT - 50.0;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        bullet_timer += dt;
        enemy_timer += dt;
            update_bullets_and_enemys(dt);
            update_player(dt);
            bz::animation_update(&enemy_animation, dt);
            main_enemy_pos = enemy_animation.C;            

        BeginDrawing();
        ClearBackground(WINDOW_COLOR);
            for (bullet_t& bullet : bullets) {
                DrawCircleV(bullet.center, BULLET_RADIUS, BULLET_COLOR);
            }
            for (bullet_t& bullet : special_bullets) {        
                DrawCircleV(bullet.center, BULLET_RADIUS, BULLET_COLOR);
            }
            for (enemy_t& enemy : enemys) {                
                DrawCircleV(enemy.center, ENEMY_RADIUS, ENEMY_COLOR);
            }
            DrawCircleV(player_pos, PLAYER_RADIUS, PLAYER_COLOR);
            DrawCircleV(main_enemy_pos, MAIN_ENEMY_RADIUS, MAIN_ENEMY_COLOR);
        EndDrawing();
    }

    CloseWindow();

}
