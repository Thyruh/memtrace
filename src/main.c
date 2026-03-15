#include "../include/memtrace.h"

int main(void) {
   int* ptr = (int*)malloc(sizeof(int)*10); // 40
   free(ptr);
   int* ptr1 = (int*)malloc(sizeof(int)*100); // 400
   int* ptr2 = (int*)malloc(sizeof(int)*100); // 400
   int* ptr3 = (int*)malloc(sizeof(int)*1000); // 4000

   free(ptr1);
   free(ptr2);

   return memtrace_exit();
}

