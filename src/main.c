#include "../include/memtrace.h"

int main(void) {
   void* ptr;
   for (int i = 1; i < 10; i++) {
      ptr = malloc(i);
   }

   ptr = malloc(1);

   ptr = malloc(2);
   return memtrace_exit();
}

