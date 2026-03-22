#define ANSI_COLOR_GREEN   "\x1b[32m"

#include "../include/memtrace.h"
#undef malloc
#undef calloc
#undef realloc 
#undef free
#include "../include/darray.h"

static inline size_t strlen_(char* p) {
   char* base = p;
   while (*p) p++;
   return p-base;
}

int print_line_from(const char *filename, int pos) {
    if (pos-- < 0) return -1;
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
            printf("%s%s%s", ANSI_COLOR_GREEN, line, ANSI_COLOR_RESET);
            found = 1;
            break;
        }
        counter++;
    }
    fclose(f);
    return found ? 0 : -1;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
DARRAY_INIT(mem_info)
DARRAY_BIND(mem_info, info)
#pragma clang diagnostic pop

size_t total   = 0;
size_t current = 0;
size_t peak    = 0;

void* memtrace_malloc(const size_t size, const char* file, const size_t line) {
   void* ptr = malloc(size);
   if (ptr != 0) {
      mem_info new;
      new.alloced = size;
      new.line = line;
      new.file = file;
      new.self = ptr;
      info_push(new);
      current += size;
      total += size;
      if (current > peak) peak = current;
   }
   return ptr;
}

void* memtrace_calloc(const size_t len, const size_t size, const char* file, const size_t line) {
   void* ptr = calloc(len, size);
   if (ptr != 0) {
      size_t total_size = len * size;
      current += total_size;
      total += total_size;
      if (current > peak) peak = current;
      mem_info new;
      new.alloced = total_size;
      new.line = line;
      new.file = file;
      new.self = ptr;
      info_push(new);
   }
   return ptr;
}

void* memtrace_realloc(void* ptr, const size_t size, const char* file, const size_t line) {
   if (!ptr) return memtrace_malloc(size, file, line);
   void* tmp = realloc(ptr, size);
   if (!tmp) return tmp;
   for (size_t i = 0; i < info.size; i++) {
      if (ptr == info_at(i).self) {
         current -= info_at(i).alloced;
         current += size;
         total -= info_at(i).alloced;
         total += size;
         if (current > peak) peak = current;
         mem_info new = info_at(i);
         new.alloced = size;
         new.self = tmp;
         new.file = file;
         new.line = line;
         info_replace(i, new);
         break;
      }
   }
   return tmp;
}

void memtrace_free(void* ptr) {
   if(!ptr) return;
   for (size_t i = 0; i < info.size; i++) {
      if (ptr == info_at(i).self) {
         current -= info_at(i).alloced;
         info_replace(i, info_at(info.size-1));
         info_pop();
         break;
      }
   }
   free(ptr);
}

int memtrace_exit(void) {
   size_t leaked = 0;

   for (size_t i = 0; i < info.size; i++) {
      mem_info alloced = info_at(i);
      leaked += alloced.alloced;
      printf("\nAllocated at %s:%zu ", alloced.file, alloced.line);
      printf("and%s lost%s: %zu\n", ANSI_COLOR_RED, ANSI_COLOR_RESET, alloced.alloced);
      print_line_from(alloced.file, alloced.line);
   }

   printf("\n========[SUMMARY]=========\n");
   printf("Leaked: %zuB\n", leaked);
   printf("Total: %zuB\n", total);
   printf("Peak: %zuB\n", peak);
   printf("==========================\n");
   total = 0;
   peak = 0;
   current = 0;
   if (leaked == 0) printf("[MEMTRACE]: No memory leaks detected.\n");
   info_free();
   return 0;
}
