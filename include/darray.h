// v0.0.1
#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define DARRAY_INIT(T)                                                                                                 \
   typedef struct {                                                                                                    \
      T* node;                                                                                                         \
      size_t size;                                                                                                     \
      size_t capacity;                                                                                                 \
   } Darray_##T;                                                                                                       \
static inline bool T##_grow(Darray_##T* darray) {                                                                      \
   size_t new_capacity = darray->capacity + 1 + darray->capacity / 2;                                                  \
   T* tmp = realloc(darray->node, sizeof(T) * new_capacity);                                                           \
   if (!tmp) return false;                                                                                             \
   darray->node = tmp;                                                                                                 \
   darray->capacity = new_capacity;                                                                                    \
   return true;                                                                                                        \
}                                                                                                                      \
static inline bool T##_push(Darray_##T* darray, T item) {                                                              \
   if (darray->size >= darray->capacity) {                                                                             \
      if (!T##_grow(darray)) return false;                                                                             \
   }                                                                                                                   \
   darray->node[darray->size++] = item;                                                                                \
   return true;                                                                                                        \
}                                                                                                                      \
static inline T T##_pop(Darray_##T* darray) {                                                                          \
   if (darray->size == 0) {                                                                                            \
      printf("\n%s%sPanic: _pop() on empty darray\n%s", ANSI_COLOR_RED, "[DARRAY] ", ANSI_COLOR_RESET);               \
      exit(1);                                                                                                         \
   }                                                                                                                   \
   return darray->node[--darray->size];                                                                                \
}                                                                                                                      \
static inline void T##_strip(Darray_##T* darray, size_t count) {                                                       \
   if (count > darray->size) darray->size = 0;                                                                         \
   else darray->size -= count;                                                                                         \
}                                                                                                                      \
static inline T T##_unsafe_at(Darray_##T* darray, size_t index) {                                                      \
   return darray->node[index];                                                                                         \
}                                                                                                                      \
static inline T T##_at(Darray_##T* darray, size_t index) {                                                             \
   if (index >= darray->size) {                                                                                        \
      printf("\n%s%sPanic: _at() access out of bounds\n%s", ANSI_COLOR_RED, "[DARRAY] ", ANSI_COLOR_RESET);            \
      exit(1);                                                                                                         \
   }                                                                                                                   \
   return darray->node[index];                                                                                         \
}                                                                                                                      \
static inline T* T##_at_ptr(Darray_##T* darray, size_t index) {                                                        \
   if (index >= darray->size) {                                                                                        \
      printf("\n%s%sPanic: _at_ptr() access out of bounds\n%s", ANSI_COLOR_RED, "[DARRAY] ", ANSI_COLOR_RESET);        \
      exit(1);                                                                                                         \
   }                                                                                                                   \
   return &darray->node[index];                                                                                        \
}                                                                                                                      \
static inline void T##_replace(Darray_##T* darray, size_t index, T new) {                                             \
   if (index >= darray->size) {                                                                                        \
      printf("\n%s%sPanic: _replace() access out of bounds\n%s", ANSI_COLOR_RED, "[DARRAY] ", ANSI_COLOR_RESET);       \
      exit(1);                                                                                                         \
   }                                                                                                                   \
   darray->node[index] = new;                                                                                          \
}                                                                                                                      \
static inline void T##_remove(Darray_##T* darray, size_t index) {                                                      \
   if (index >= darray->size) {                                                                                        \
      printf("\n%s%sPanic: _remove() access out of bounds\n%s", ANSI_COLOR_RED, "[DARRAY] ", ANSI_COLOR_RESET);        \
      exit(1);                                                                                                         \
   }                                                                                                                   \
   for (size_t i = index; i < darray->size - 1; i++) {                                                                 \
      darray->node[i] = darray->node[i + 1];                                                                           \
   }                                                                                                                   \
   darray->size--;                                                                                                     \
}                                                                                                                      \
static inline bool T##_reserve(Darray_##T* darray, size_t block) {                                                     \
   if (block <= darray->capacity) return true;                                                                         \
   T* tmp = realloc(darray->node, sizeof(T) * block);                                                                  \
   if (!tmp) return false;                                                                                             \
   darray->node = tmp;                                                                                                 \
   darray->capacity = block;                                                                                           \
   return true;                                                                                                        \
}                                                                                                                      \
static inline bool T##_shrink(Darray_##T* darray) {                                                                    \
   if (darray->size == darray->capacity) return true;                                                                  \
   if (darray->size == 0) {                                                                                            \
      free(darray->node);                                                                                              \
      darray->node = NULL;                                                                                             \
      darray->capacity = 0;                                                                                            \
      return true;                                                                                                     \
   }                                                                                                                   \
   T* tmp = realloc(darray->node, sizeof(T) * darray->size);                                                           \
   if (!tmp) return false;                                                                                             \
   darray->node = tmp;                                                                                                 \
   darray->capacity = darray->size;                                                                                    \
   return true;                                                                                                        \
}                                                                                                                      \
static inline bool T##_append(Darray_##T* darray, T new) {                                                             \
   if (darray->size >= darray->capacity) {                                                                             \
      if (!T##_grow(darray)) return false;                                                                             \
   }                                                                                                                   \
   darray->size++;                                                                                                     \
   for (size_t i = darray->size - 1; i > 0; i--) {                                                                     \
      darray->node[i] = darray->node[i - 1];                                                                           \
   }                                                                                                                   \
   darray->node[0] = new;                                                                                              \
   return true;                                                                                                        \
}                                                                                                                      \
static inline bool T##_insert_at(Darray_##T* darray, size_t index, T item) {                                           \
   if (index > darray->size) {                                                                                         \
      printf("\n%s%sPanic: _insert_at() index out of bounds\n%s", ANSI_COLOR_RED, "[DARRAY] ", ANSI_COLOR_RESET);      \
      exit(1);                                                                                                         \
   }                                                                                                                   \
   if (darray->size >= darray->capacity) {                                                                             \
      if (!T##_grow(darray)) return false;                                                                             \
   }                                                                                                                   \
   darray->size++;                                                                                                     \
   for (size_t i = darray->size - 1; i > index; i--) {                                                                 \
      darray->node[i] = darray->node[i - 1];                                                                           \
   }                                                                                                                   \
   darray->node[index] = item;                                                                                         \
   return true;                                                                                                        \
}                                                                                                                      \
static inline Darray_##T T##_new(void) {                                                                               \
   return (Darray_##T) {NULL, 0, 0};                                                                                   \
}                                                                                                                      \
static inline void T##_free(Darray_##T* darray) {                                                                      \
   free(darray->node);                                                                                                 \
   *darray = T##_new();                                                                                                \
}

#define DARRAY_BIND(T, varname)                                                                                        \
   Darray_##T varname = {NULL, 0, 0};                                                                                  \
   static inline bool varname##_push(T item)                 { return T##_push(&varname, item); }                      \
   static inline T    varname##_pop(void)                    { return T##_pop(&varname); }                             \
   static inline void varname##_strip(size_t count)          { T##_strip(&varname, count); }                           \
   static inline T    varname##_unsafe_at(size_t idx)        { return T##_unsafe_at(&varname, idx); }                  \
   static inline T    varname##_at(size_t idx)               { return T##_at(&varname, idx); }                         \
   static inline T*   varname##_at_ptr(size_t idx)           { return T##_at_ptr(&varname, idx); }                     \
   static inline bool varname##_reserve(size_t block)        { return T##_reserve(&varname, block); }                  \
   static inline bool varname##_shrink(void)                 { return T##_shrink(&varname); }                          \
   static inline void varname##_replace(size_t index, T new) { T##_replace(&varname, index, new); }                    \
   static inline void varname##_remove(size_t index)         { T##_remove(&varname, index); }                          \
   static inline bool varname##_append(T new)                { return T##_append(&varname, new); }                     \
   static inline bool varname##_insert_at(size_t index, T item) { return T##_insert_at(&varname, index, item); }       \
   static inline void varname##_free(void)                   { T##_free(&varname); }

