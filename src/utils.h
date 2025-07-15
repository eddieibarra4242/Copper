#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "scanner.h"

typedef union ConstantValue {
  uint64_t bits;
} Constant;

/**
 * Transforms an array into a max-heap.
 *
 * @param array The array to transform.
 * @param nelements The number of elements in the array.
 */
void heapify(uint64_t *array, size_t nelements);

/**
 * Sorts an array using heap sort algorithm.
 *
 * @param array The array to sort.
 * @param nelements The number of elements in the array.
 */
void heap_sort(uint64_t *array, size_t nelements);

/**
 * Evaluates a constant token and returns its value.
 *
 * @param token The token to evaluate.
 * @return The value of the token.
 */
Constant eval_token(Token *token);
