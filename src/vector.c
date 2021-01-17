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

float vector_distance(vector a, vector b){

    return sqrt(((b.x - a.x) * (b.x - a.x)) + ((b.y - a.y) * (b.y - a.y)));
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

int partition(float** array, int low, int high){

    int pivot = array[high][1];
    int i = low - 1;

    for(int j = low; j <= high - 1; j++){

        if(array[j][1] < pivot){

            i++;
            int temp_key = array[i][0];
            int temp_val = array[i][1];
            array[i][0] = array[j][0];
            array[i][1] = array[j][1];
            array[j][0] = temp_key;
            array[j][1] = temp_val;
        }
    }

    int temp_key = array[i + 1][0];
    int temp_val = array[i + 1][1];
    array[i + 1][0] = array[high][0];
    array[i + 1][1] = array[high][1];
    array[high][0] = temp_key;
    array[high][1] = temp_val;

    return i + 1;
}

void quicksort(float** array, int low, int high){

    if(low < high){

        int pi = partition(array, low, high);

        quicksort(array, low, pi - 1);
        quicksort(array, pi + 1, high);
    }
}

int max(int a, int b){

    return a > b ? a : b;
}
