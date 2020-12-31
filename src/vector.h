#pragma once

#define PI 3.14159265358979323846

typedef struct vector{
    float x;
    float y;
} vector;

extern const vector ZERO_VECTOR;

vector vector_sum(vector a, vector b);
vector vector_sub(vector a, vector b);
vector vector_mult(vector a, float b);
float vector_magnitude(vector a);
vector vector_scale(vector a, float b);
vector vector_rotate(vector a, float b);

int max(int a, int b);
