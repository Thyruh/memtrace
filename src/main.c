#include "../include/memtrace.h"

int main(void) {
   int* ptr1 = malloc(1);
   void* ptr2 = malloc(1);
   void* ptr3 = malloc(1);
   void* ptr4 = malloc(10);

   void* ptr7 = realloc(ptr1, 10);

   return memtrace_exit();
}

