#include "../include/memtrace.h"

int main(void) {
   int* ptr = (int*)malloc(sizeof(int)*10); // 40
   free(ptr);
   int* ptr1 = (int*)malloc(sizeof(int)*100); // 400
   free(ptr1);
   int* ptr2 = (int*)malloc(sizeof(int)*100); // 400
   free(ptr2);
   int* ptr3 = (int*)malloc(sizeof(int)*1000); // 4000
   free(ptr3);

   return memtrace_exit();
}

