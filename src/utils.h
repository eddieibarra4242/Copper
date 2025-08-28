#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "scanner.h"

typedef union ConstantValue {
  uint64_t bits;
} Constant;

struct allocation {
  void *address;
  struct allocation *next;
};

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

/**
 * Allocates memory or raises a critical error if allocation fails.
 * Note: This function will also track all allocations made in provided
 * allocation list.
 *
 * @param size The size of the memory to allocate.
 * @param allocation_list A pointer to the allocation list to update. (NULL if
 * not tracking)
 * @return A pointer to the allocated memory.
 */
void *allocate_or_error(size_t size, struct allocation **allocation_list);
