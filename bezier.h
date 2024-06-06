#ifndef BEZIER_CURVE_H
#define BEZIER_CURVE_H
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <vector>


namespace bz {


    enum TBasicFunction {
        Normal,
        Quadratic,
        Cubic,
        SquareRoot,
        QuadraticEasyOut,
        Parabola   
    };


    typedef struct bezier_progress {
        double time_to_complete = 0.0; // Tempo em segundos para terminar a animação
        double time_count = 0.0; // Tempo decorrido desde o inicio da animação
        bool reverse = false; // Flag que diz se a animação está indo no sentido esquerda -> direita ou direita -> esquerda
        bool loop = false; // Flag que diz se a animação deve ser feita em loop ou apenas uma vez
    } bezier_progress_t;


    typedef struct bezier_animation {        
        std::vector<Vector2> control_points{}; // Pontos de controle da animação    
        Vector2 C; // Ponto que vai de start até target atraves do tempo t     
        Vector2 lastMovement = {0}; // Posição atual de C no tempo t menos posição de C no tempo t - 1
        bezier_progress_t progress{}; // Marca o progresso da animação
        TBasicFunction t_function{}; // Função a ser aplicada ao valor de t
    } bezier_animation_t;


    bezier_animation_t* bezier_animation_create(
        const double time_to_complete,
        const Vector2 start,
        const Vector2 target,    
        const bool loop,
        const TBasicFunction t_function
    ) {
        bezier_animation_t* animation = new bezier_animation_t{};    
        animation->control_points.push_back(start);        
        animation->control_points.push_back(target);        
        animation->t_function = t_function;
        animation->progress.time_to_complete = time_to_complete;        
        animation->progress.loop = loop;
        return animation;
    }


    void bezier_animation_destroy(bezier_animation_t* animation) {    
        delete animation;
    }


    /**
     * Adiciona um ponto de controle a animação.  
    */
    void add_control_point(bezier_animation_t* animation, const Vector2 point) {    
        const Vector2 origin = Vector2Zero();
        const float distanceFromOrirgin = Vector2Distance(point, origin);
        for (int i = 0; i < animation->control_points.size(); i++) {
            if (Vector2Distance(animation->control_points[i], origin) >= distanceFromOrirgin) {
                animation->control_points.insert(animation->control_points.begin() + i, point);
                return;
            }
        }
    }

    void remove_point(bz::bezier_animation_t* animation, const std::size_t i) {
        if (i != 0 && i != animation->control_points.size() - 1) {
            animation->control_points.erase(animation->control_points.begin() + i);
        }
    }

    void insert_control_point(bz::bezier_animation_t* animation, const Vector2 point, std::size_t i) {
        animation->control_points.insert(animation->control_points.begin() + i + 1, point);
    }

    /**
     * Calcula o ponto a ser adicionado a partir da lerp entre o ponto inicial e o final e 
     * adiciona aos pontos de controle 
    */
    void add_control_point_lerp(
        bezier_animation_t* animation,
        const double ammount
    ) {
        Vector2 point = Vector2Lerp(animation->control_points[0], animation->control_points[animation->control_points.size()-1], ammount);
        bz::add_control_point(animation, point);
    }


    void push_back_cotrol_point(bezier_animation_t* animation, const Vector2 point) {
        animation->control_points.push_back(point);
    }


    void push_front_control_point(bezier_animation_t* animation, const Vector2 point) {
        animation->control_points.insert(animation->control_points.begin(), point);
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

    double update_progress(bz::bezier_animation_t* animation, const float dt) {
        animation->progress.time_count += dt;
        double t = animation->progress.time_count / animation->progress.time_to_complete;
        t = animation->progress.reverse ? 1.0 - t : t;
        if (animation->progress.loop && (t > 1.0 || t < 0.0)) {
            animation->progress.reverse = animation->progress.loop ? !animation->progress.reverse : false;
            animation->progress.time_count = 0.0;
        } 
        return bz::apply_t_function(animation->t_function, t);
    }    

    /**
     * Atualiza a posição do ponto C em relação a um tempo t
    */
    void animation_update(bezier_animation_t* animation, const float dt) {                
        const double t = bz::update_progress(animation, dt);
        const int n = animation->control_points.size() - 1;
        const Vector2 oldC = animation->C;
        animation->C = Vector2Zero();
        for (int k = 0; k < n+1; k++) {
            const double b_coeff = binomial_coefficient(n, k);
            const double bernstein_poly = b_coeff * pow(t, k) * pow(1.0 - t, n - k);
            animation->C = Vector2Add(animation->C, Vector2Scale(animation->control_points[k], bernstein_poly));
        }        
        animation->lastMovement = Vector2Subtract(animation->C, oldC);
    }    

}  // namespace bz




#endif