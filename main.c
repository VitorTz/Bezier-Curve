#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>


#define CIRCLE_RADIUS 12.0
#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720
#define WINDOW_TITLE "Bezier Curve"
#define PADDING 50.0
#define MAX_POINTS 40
#define BG_COLOR (GetColor(0x181818ff))


int mouse_dragged = 0;
Vector2* last_circle_dragged = NULL;


typedef struct bezier_animation {    
    Vector2 points[MAX_POINTS];
    Vector2 B;
    size_t num_points;
} bezier_animation_t;


typedef struct animation_timer {
    double time_end;
    double progrees;
    double time_count;
    int complete;
} animation_timer_t;


bezier_animation_t* bezier_animation_create(
    const double a1,
    const double a2,
    const double b1,
    const double b2,
    size_t num_points
) {
    if (num_points + 2 > MAX_POINTS ) {
        num_points = MAX_POINTS - 2;
        printf("[Aviso]: Número de pontos excede o número máximo (%d), saturando o número em %d pontos.\n", MAX_POINTS, MAX_POINTS);
    } else if (num_points <= 1) {
        num_points = 1;
        printf("[Aviso]: Número de pontos não pode ser menor do que 1. Alterando o número de pontos para 1.\n");
    }

    const double step = 1.0 / (num_points + 1.0);
    printf("[Nova Bezier Animation] [Points = %ld] [Step = %.2f]\n", num_points, step);

    bezier_animation_t* p = (bezier_animation_t*) malloc(sizeof(bezier_animation_t));
    p->num_points = num_points;
    p->points[0].x = a1;
    p->points[0].y = a2;
    p->points[num_points+1].x = b1;
    p->points[num_points+1].y = b2;
    p->B.x = 0.f;
    p->B.y = 0.f;
    double s = step;
    for (int i = 1; i < num_points + 1; i++) {
        p->points[i] = Vector2Lerp(p->points[0], p->points[num_points+1], s);
        s += step;
    }
    return  p;
}


void bezier_handle_mouse_dragged(bezier_animation_t* animation) {
    const Vector2 mouse_pos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        mouse_dragged = 1;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        mouse_dragged = 0;
        last_circle_dragged = NULL;
        return;
    }

    if (mouse_dragged == 1) {
        for (int i = 0; i < animation->num_points + 2; i++) {
            if (CheckCollisionPointCircle(mouse_pos, animation->points[i], CIRCLE_RADIUS)) {
                last_circle_dragged = &animation->points[i];
                break;
            }
        }
    }

    if (mouse_dragged && last_circle_dragged != NULL) {
        *last_circle_dragged = mouse_pos;
    }

}


void bezier_animation_destroy(bezier_animation_t* animation) {
    free(animation);
}


void bezier_draw_lines(bezier_animation_t* animation) {
    
    for (int i = 0; i < animation->num_points + 1; i++) {
        DrawLineV(
            animation->points[i], 
            animation->points[i+1],
            BROWN
        );
    }
}


void bezier_draw_num(bezier_animation_t* animation, const int num) {
    char n = num + '0';
    DrawText(&n, animation->points[num].x - 4, animation->points[num].y - 8, 20, WHITE);
}


void bezier_draw_points(bezier_animation_t* animation) {    
    DrawCircleV(animation->points[0], CIRCLE_RADIUS, RED);
    bezier_draw_num(animation, 0);
    for (int i = 1; i < animation->num_points + 1; i++) {        
        DrawCircleV(animation->points[i], CIRCLE_RADIUS, BROWN);    
        bezier_draw_num(animation, i);
    }
    DrawCircleV(animation->points[animation->num_points+1], CIRCLE_RADIUS, RED);
    bezier_draw_num(animation, animation->num_points+1);
}


void print_vector(Vector2 v) {
    printf("Vec(%.2f, %.2f)\n", v.x, v.y);
}


int factorial(int n) {
    int r = 1;
    for (int i = 1; i <= n; i++) {
        r *= i;
    }
    return r;
}

int binomial_coefficient(int n, int k) {    
    double res = 1;
    for (int i = 1; i <= k; ++i)
        res = res * (n - k + i) / i;
    return (int)(res + 0.01);
}

Vector2 bezier_path_aux(const int n, const int i, const double t, const Vector2 p) {
    double b = binomial_coefficient(n, i);
    double c = pow((1.0 - t), n - i);
    double d = pow(t, i);
    double e = b * c * d;
    return Vector2Scale(p, e);
}


void bezier_path(
    bezier_animation_t* animation,     
    const double t
) {
    const int n = animation->num_points + 1;
    animation->B.x = animation->B.y = 0.f;
    for (int k = 0; k < n+1; k++) {
        const double b_coeff = binomial_coefficient(n, k);
        const double bernstein_poly = b_coeff * pow(t, k) * pow(1.0 - t, n - k);
        animation->B = Vector2Add(animation->B, Vector2Scale(animation->points[k], bernstein_poly));
    }
    DrawCircleV(animation->B, CIRCLE_RADIUS, BLUE);
}


void update_timer(animation_timer_t* timer) {
    timer->time_count += GetFrameTime();
    timer->progrees = timer->time_count / timer->time_end;    
    if (timer->progrees >= 1.0) {
        timer->time_count = 0.0;
    }
}


int main(int argc, char const *argv[]) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    
    bezier_animation_t* animation = bezier_animation_create(PADDING, PADDING, SCREEN_WIDTH - PADDING, SCREEN_HEIGHT - PADDING, 4);
    animation_timer_t timer = {0};
    timer.time_end = 5.0;        
    while (!WindowShouldClose()) {        
        BeginDrawing();
        ClearBackground(BG_COLOR);            
            update_timer(&timer);
            bezier_handle_mouse_dragged(animation);            
            bezier_draw_points(animation);
            bezier_draw_lines(animation);            
            bezier_path(animation, timer.progrees);
            if (IsKeyPressed(KEY_SPACE)) {
                timer.time_count = 0.0;                
            }
        EndDrawing();        
    }

    bezier_animation_destroy(animation);
    CloseWindow();
    return 0;
}
