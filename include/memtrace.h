#include <stdio.h>
#include <stdlib.h>

typedef struct {
   size_t alloced;
   size_t line;
   const char* file;
   void* self;
} mem_info;

int   memtrace_exit(void);
void  memtrace_free(void* ptr);
void* memtrace_malloc(const size_t size, const char* file, size_t line);
void* memtrace_realloc(void* ptr, const size_t size, const char* file, size_t line);
void* memtrace_calloc(const size_t len, const size_t size, const char* file, size_t line);

#define malloc(size)           (memtrace_malloc((size), __FILE__, __LINE__))
#define realloc(ptr, size)     (memtrace_realloc((ptr), (size), __FILE__, __LINE__))
#define calloc(length, size)   (memtrace_calloc((length), (size), __FILE__, __LINE__))
#define free(ptr)              (memtrace_free(ptr))
