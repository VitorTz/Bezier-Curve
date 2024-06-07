#ifndef BEZIER_CURVE_H
#define BEZIER_CURVE_H
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <vector>
#include <cassert>
#include <algorithm>


namespace bz {


    enum TBasicFunction {
        Normal,
        Quadratic,
        Cubic,
        SquareRoot,
        QuadraticEasyOut,
        Parabola   
    };

    typedef struct bezier_animation {        
        std::vector<Vector2> control_points; // Pontos de controle da animação    
        Vector2 C; // Ponto que vai de start até target atraves do tempo t             
        double time_to_complete = 0.0;
        double time_count = 0.0;
        double t = 0.0;
        bool reverse = false;
        bool loop = false;
        TBasicFunction t_function = bz::TBasicFunction::Normal; // Função a ser aplicada ao valor de t
        bezier_animation(const Vector2 start, const Vector2 end) {
            control_points.push_back(start);
            control_points.push_back(end);
        }
        bezier_animation() = default;
    } bezier_animation_t;

    void sort_control_points(bz::bezier_animation_t* animation) {
        if (animation->control_points.empty()) {
            return;
        }
        const Vector2 start = animation->control_points[0];
        std::sort(
            animation->control_points.begin(),
            animation->control_points.end(),
            [start](const Vector2 l, const Vector2 r) {                
                return Vector2Distance(l, start) < Vector2Distance(r, start);
            }
        );
    }

    void push_back_control_point(bz::bezier_animation_t* animation, const Vector2 point) {
        animation->control_points.push_back(point);
    }

    void push_front_control_point(bz::bezier_animation_t* animation, const Vector2 point) {
        animation->control_points.insert(animation->control_points.begin(), point);
    }

    void insert_control_point(
        bz::bezier_animation_t* animation, 
        const Vector2 point, 
        const std::size_t i
    ) {
        assert(i < animation->control_points.size());
        animation->control_points.insert(animation->control_points.begin() + i, point);
    }

    void remove_control_point(bz::bezier_animation_t* animation, const std::size_t i) {
        assert(i < animation->control_points.size());
        animation->control_points.erase(animation->control_points.begin() + i);        
    }

    void change_end_point(bz::bezier_animation_t* animation, const Vector2 point) {
        assert(animation->control_points.empty() == false);
        animation->control_points[animation->control_points.size() - 1] = point;
    }

    /**
     * Calcula o ponto a ser adicionado a partir da lerp entre o ponto inicial e o final e 
     * adiciona aos pontos de controle 
    */
    void add_control_point_lerp(
        bezier_animation_t* animation,
        const double ammount
    ) {
        assert(
            animation->control_points.size() > 1 && "Animação possui 1 ou menos pontos de controle!"
        );                        
        animation->control_points.push_back(
            Vector2Lerp(animation->control_points.front(), animation->control_points.back(), ammount)
        );
        bz::sort_control_points(animation);
    }

    int binomial_coefficient(int n, int k) {    
        double res = 1;
        for (int i = 1; i <= k; ++i)
            res = res * (n - k + i) / i;
        return (int)(res + 0.01);
    }
    

    double apply_t_function(const bz::TBasicFunction f, const double t) {
        switch (f) {
            case TBasicFunction::Quadratic:
                return t * t;                
            case TBasicFunction::Cubic:
                return t * t * t;                
            case TBasicFunction::SquareRoot:
                return std::sqrt(t);
                break;
            case TBasicFunction::QuadraticEasyOut:
                return 1.0 - (1.0 - t) * (1.0 -t);                
            case TBasicFunction::Parabola:
                return std::pow(4.0 * t * (1.0 - t), 2);                
            default:            
                break;  
        }
        return t;
    }

    bool is_animation_complete(bz::bezier_animation_t* animation) {
        return (animation->time_count >= animation->time_to_complete);
    }

    double update_progress(bz::bezier_animation_t* animation, const float dt) {
        animation->time_count += dt;        
        animation->t = animation->time_count / animation->time_to_complete;
        animation->t = animation->reverse ? 1.0 - animation->t : animation->t;
        if (animation->loop && (animation->t > 1.0 || animation->t < 0.0)) {
            animation->reverse = animation->loop ? !animation->reverse : false;
            animation->time_count = 0.0;
        } 
        return bz::apply_t_function(animation->t_function, animation->t);
    }    

    /**
     * Atualiza a posição do ponto C em relação a um tempo t
    */
    void animation_update(
        bezier_animation_t* animation, 
        const float dt
    ) {                
        const double t = bz::update_progress(animation, dt);
        const int n = animation->control_points.size() - 1;        
        animation->C = Vector2Zero();
        for (int k = 0; k < n+1; k++) {
            const double b_coeff = binomial_coefficient(n, k);
            const double bernstein_poly = b_coeff * pow(t, k) * pow(1.0 - t, n - k);
            animation->C = Vector2Add(animation->C, Vector2Scale(animation->control_points[k], bernstein_poly));
        }        
    }

    void animation_update_follows_target(
        bz::bezier_animation_t* animation, 
        const float dt,
        const Vector2 target
    ) {
        animation->control_points[animation->control_points.size() - 1] = target;
        const double t = bz::update_progress(animation, dt);
        const int n = animation->control_points.size() - 1;        
        animation->C = Vector2Zero();
        for (int k = 0; k < n+1; k++) {
            const double b_coeff = binomial_coefficient(n, k);
            const double bernstein_poly = b_coeff * pow(t, k) * pow(1.0 - t, n - k);
            animation->C = Vector2Add(animation->C, Vector2Scale(animation->control_points[k], bernstein_poly));
        }     
    }

}  // namespace bz




#endif