#include <stdio.h>

typedef struct {
   size_t initial_size;
   size_t bytes_alloced;
   size_t line;
   const char* file;
   void* self;
} mem_info;

int   memtrace_exit(void);
void  memtrace_free(void* ptr);
void* memtrace_malloc(const size_t size, const char* file, size_t line);
void* memtrace_realloc(void* ptr, const size_t size, const char* file, size_t line);

#define malloc(size)           (memtrace_malloc((size), __FILE__, __LINE__))
#define realloc(ptr, size)     (memtrace_realloc((ptr), (size), __FILE__, __LINE__))
#define free(ptr)              (memtrace_free(ptr))
