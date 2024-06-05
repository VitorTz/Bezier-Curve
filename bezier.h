#ifndef BEZIER_CURVE_H
#define BEZIER_CURVE_H
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <vector>


enum LerpFT {
    Normal,
    Quadratic,
    Cubic,
    SquareRoot,
    QuadraticEasyOut,
    Parabola,    
};


typedef struct bezier_progress {
    double time_to_complete; // Tempo em segundos para terminar a animação
    double time_count; // Tempo decorrido desde o inicio da animação
    bool reverse; // Flag que diz se a animação está indo no sentido esquerda -> direita ou direita -> esquerda
    bool loop; // Flag que diz se a animação deve ser feita em loop ou apenas uma vez
} bezier_progress_t;


typedef struct bezier_animation {        
    std::vector<Vector2> control_points{}; // Pontos de controle da animação    
    Vector2 C{}; // Ponto que vai de start até target atraves do tempo t     
    bezier_progress_t progress{}; // Marca o progresso da animação
    LerpFT lerp_type{};
} bezier_animation_t;


bezier_animation_t* bezier_animation_create(
    const double time_to_complete,
    const Vector2 start,
    const Vector2 target,    
    const bool loop,
    const LerpFT lerp
) {
    bezier_animation_t* animation = new bezier_animation_t{};    
    animation->control_points.push_back(start);        
    animation->control_points.push_back(target);
    animation->C = {0};    
    animation->lerp_type = lerp;
    animation->progress.time_to_complete = time_to_complete;
    animation->progress.time_count = 0.0;
    animation->progress.reverse = false;
    animation->progress.loop = loop;
    return animation;
}


void bezier_animation_destroy(bezier_animation_t* animation) {    
    free(animation);
}


/**
 * Adiciona um ponto de controle a animação.  
*/
void bezier_animation_add_control_point(bezier_animation_t* animation, const Vector2 point) {
    Vector2 last = animation->control_points.back();
    animation->control_points.pop_back();
    animation->control_points.push_back(point);
    animation->control_points.push_back(last);
}


void bezier_animation_add_control_point(
    bezier_animation_t* animation,
    const double step
) {
    Vector2 point = Vector2Lerp(animation->control_points[0], animation->control_points[animation->control_points.size()-1], step);
    bezier_animation_add_control_point(animation, point);
}


void bezier_animation_change_target(bezier_animation_t* animation, const Vector2 target) {
    animation->control_points[animation->control_points.size()-1] = target;    
}


int binomial_coefficient(int n, int k) {    
    double res = 1;
    for (int i = 1; i <= k; ++i)
        res = res * (n - k + i) / i;
    return (int)(res + 0.01);
}


/**
 * Atualiza a posição do ponto C em relação a um tempo t
*/
void bezier_animation_update(bezier_animation_t* animation, const float dt) {
    // update t
    animation->progress.time_count += dt;
    double t = animation->progress.time_count / animation->progress.time_to_complete;
    t = animation->progress.reverse ? 1.0 - t : t;
    if (animation->progress.loop && (t > 1.0 || t < 0.0)) {
        animation->progress.reverse = animation->progress.loop ? !animation->progress.reverse : false;
        animation->progress.time_count = 0.0;
    }    

    switch (animation->lerp_type) {
        case LerpFT::Quadratic:
            t = t * t;
            break;
        case LerpFT::Cubic:
            t = t * t * t;
            break;
        case LerpFT::SquareRoot:
            t = std::sqrt(t);
            break;
        case LerpFT::QuadraticEasyOut:
            t = 1.0 - (1.0 - t) * (1.0 -t);
            break;
        case LerpFT::Parabola:
            t = std::pow(4.0 * t * (1.0 - t), 2);
            break;            
        default:
            break;  
    }
    // update point C pos    
    const int n = animation->control_points.size() - 1;
    animation->C = Vector2Zero();
    for (int k = 0; k < n+1; k++) {
        const double b_coeff = binomial_coefficient(n, k);
        const double bernstein_poly = b_coeff * pow(t, k) * pow(1.0 - t, n - k);
        animation->C = Vector2Add(animation->C, Vector2Scale(animation->control_points[k], bernstein_poly));
    }        
}


#endif