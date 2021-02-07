#pragma once

#include <stdlib.h>

/*
 * This file provides functions for reusable dynamic memory code to be used like a C++ style vector
 * It uses void type and a little bit of pointer math in order to be type-agnostic
 *
 * Note that currently it does not shrink the memory automatically at any point. The vector doubles
 * when it needs to expand capacity but never decides to shrink at any point
 */

void vector_array_push(void** vector, void* to_push, int* count, int* capacity, size_t unit_size);
void vector_array_delete(void* vector, int index, int* count, size_t unit_size);
