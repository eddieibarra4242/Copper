#include "utils.h"

void swap(uint64_t *array, size_t a, size_t b) {
  uint64_t temp = array[a];
  array[a] = array[b];
  array[b] = temp;
}

void sink(uint64_t *array, size_t nelements, size_t root) {
  size_t left_child = root * 2 + 1;

  if (left_child >= nelements)
    return;

  if (array[root] < array[left_child]) {
    swap(array, root, left_child);
    sink(array, nelements, left_child);
  }

  size_t right_child = root * 2 + 2;

  if (right_child >= nelements)
    return;

  if (array[root] < array[right_child]) {
    swap(array, root, right_child);
    sink(array, nelements, right_child);
  }
}

void heapify(uint64_t *array, size_t nelements) {
  size_t start = nelements / 2 + 1;

  while (start != 0) {
    start--;
    sink(array, nelements, start);
  }
}

void heap_sort(uint64_t *array, size_t nelements) {
  heapify(array, nelements);

  for (size_t i = nelements - 1; i != 0; i--) {
    swap(array, i, 0);
    sink(array, i, 0);
  }
}
