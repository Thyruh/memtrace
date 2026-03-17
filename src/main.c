#include "../include/memtrace.h"

int main(void) {
   void* ptr;
   for (int i = 1; i < 1000; i++) {
      ptr = malloc(i);
   }

   return memtrace_exit();
}

