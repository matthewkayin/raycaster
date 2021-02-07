#include "vector_array.h"

#include <stdio.h>
#include <string.h>

void vector_array_push(void** vector, void* to_push, int* count, int* capacity, size_t unit_size){

    if(*count == *capacity){

        *capacity *= 2;
        (*vector) = realloc((*vector), unit_size * *capacity);
    }

    memcpy(*vector + (unit_size * *count), to_push, unit_size);
    (*count)++;
}

void vector_array_delete(void* vector, int index, int* count, size_t unit_size){

    if(index >= *count){

        printf("Error! Tried to delete vector item index %i out of size %i; index out of bounds\n", index, *count);
    }

    for(int i = index; i < (*count) - 1; i++){

        memcpy(vector + (unit_size * i), vector + (unit_size * (i + 1)), unit_size);
    }
    (*count)--;
}
