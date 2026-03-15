#include "../include/memtrace.h"
#undef malloc
#undef free
#include "../include/darray.h"

DARRAY_INIT(mem_info)
DARRAY_BIND(mem_info, info)

size_t current = 0;
size_t peak    = 0;

void* memtrace_malloc(const size_t size, const char* file, size_t line) {
   void* ptr = malloc(size);
   if (ptr != 0) {
      mem_info new;
      new.bytes_alloced = size;
      new.initial_size = size;
      new.line = line;
      new.file = file;
      new.self = ptr;
      info_push(new);
      current += size;
      if (current > peak) peak = current;
   }
   return ptr;
}

// void* memtrace_realloc(void* ptr, const size_t size, char* file, size_t line) { // TODO finish the realloc
//    return NULL;
// }

void memtrace_free(void* ptr) {
   for (size_t i = 0; i < info.size; i++) {
      if (ptr == info_at(i).self) {
         current -= info_at(i).initial_size;
         mem_info new = info_at(i);
         new.bytes_alloced = 0;
         info_replace(i, new);
         // TODO: Introduce a _replace(size_t index, T new_node) and _remove(size_t index) methods in darray to rewrite nodes
         // Maybe _append(T new_node) while we are at it
      }
   }
   free(ptr);
}

int memtrace_exit(void) { // to return from main
   size_t leaked = 0;
   size_t total = 0;
   for (size_t i = 0; i < info.size; i++) {
      mem_info alloced = info_at(i);
      leaked += alloced.bytes_alloced;
      total += info_at(i).initial_size;

      if (alloced.bytes_alloced > 0) {
         printf("\nAllocated at %s:%zu\n", alloced.file, alloced.line); // TODO print the line contents
         printf("Bytes allocated and lost: %zu\n", alloced.bytes_alloced);
      }
   }

   printf("\nLeaked: %zuB\n", leaked);
   printf("Total: %zuB\n", total); 
   printf("Peak: %zuB\n", peak);

   if (leaked == 0) printf("[MEMTRACE]: No memory leaks detected.\n");

   info_free();
   return 0;
}
