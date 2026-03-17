#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_GREEN   "\x1b[32m"

#include "../include/memtrace.h"
#undef malloc
#undef free
#include "../include/darray.h"

size_t strlen_(char* p) {
   char* base = p;
   while (*p != 0) p++;
   return p-base;
}

int print_line_from(const char *filename, int pos) {
    if (pos < 0) return -1;
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    char line[256];
    int counter = 0;
    int found = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        if (counter == pos) {
            size_t len = strlen_(line);
            if (len == sizeof(line) - 1 && line[len - 1] != '\n') {
                fclose(f);
                return -2;
            }
            printf("%s%s%s",ANSI_COLOR_GREEN, line, ANSI_COLOR_RESET);
            found = 1;
            break;
        }
        counter++;
    }
    fclose(f);
    return found ? 0 : -1;
}

// DARRAY_INIT(int)
// DARRAY_BIND(int, non_repeating_lines)
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
      }
   }
   free(ptr);
}

int memtrace_exit(void) { // to return from main
   size_t leaked = 0;
   size_t total = 0;
   for(size_t i = 0; i < info.size; i++) {
      mem_info alloced = info_at(i);
      leaked += alloced.bytes_alloced;
      total  += alloced.initial_size;
      // TODO check for loops to prevent N consecutive "lost x bytes at main.c:11"

      if (alloced.bytes_alloced > 0) {
         printf("\nAllocated at %s:%zu", alloced.file, alloced.line);
         printf(" and lost: %zu\n", alloced.bytes_alloced);
         print_line_from(alloced.file, alloced.line-1);
      }
   }

   printf("\nLeaked: %zuB\n", leaked);
   printf("Total: %zuB\n", total); 
   printf("Peak: %zuB\n", peak);

   if (leaked == 0) printf("[MEMTRACE]: No memory leaks detected.\n");

   info_free();
   return 0;
}
