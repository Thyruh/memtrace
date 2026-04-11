#include "../include/memtrace.h"

int main(void) {
   char* ptr1 = calloc(5, sizeof(int));

   char* ptr2 = malloc(5*sizeof(int));
   free(ptr2);
   void* ptr3 = malloc(30);

   void* ptr5 = realloc(ptr1, 100);

   printf("%p", ptr3);

   free(ptr5);

   return memtrace_exit();
}
