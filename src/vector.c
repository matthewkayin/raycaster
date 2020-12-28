#include "vector.h"

#include <math.h>

const vector ZERO_VECTOR = (vector){ .x = 0, .y = 0 };

vector vector_sum(vector a, vector b){

    return (vector){ .x = a.x + b.x, .y = a.y + b.y };
}

vector vector_sub(vector a, vector b){

    return (vector){ .x = a.x - b.x, .y = a.y - b.y };
}

vector vector_mult(vector a, float b){

    return (vector){ .x = a.x * b, .y = a.y * b };
}

float vector_magnitude(vector a){

    return (a.x * a.x) + (a.y * a.y);
}

vector vector_scale(vector a, float b){

    float old_magnitude = vector_magnitude(a);
    if(old_magnitude == 0){

        return a;
    }

    float scale = b / old_magnitude;
    return (vector){ .x = a.x * scale, .y = a.y * scale };
}

vector vector_rotate(vector a, float b){

    return (vector){ .x = (a.x * cos(b)) - (a.y * sin(b)), .y = (a.x * sin(b)) + (a.y * cos(b)) };
}
