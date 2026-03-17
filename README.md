## Memory Trace - is a nano library with a valgrind-like functionality and dead easy API with three functions.
- I recommend including the memtrace.h header as the last one, otherwise it might disrupt how other mallocs in other libraries work.

# 1: memtrace_malloc():
memtrace_malloc() highjacks malloc() and keeps the details about the allocation in a dynamic array of mem_info objects.
The default usage is through a macro wrapper malloc() for \_\_LINE\_\_ and \_\_FILE\_\_ details for the logging system.
# 2: memtrace_free():
memtrace_free() highjacks free() deleting the details about the allocation of provided pointers.
# 3: memtrace_exit():
memtrace_exit() by design is a function that you return from main. It reads all the previously compiled information such as line,
file, size and a raw copy of the pointer itself as an identification token about every single allocation and outputs the summary
of all the important information.
# To compile use nob: 

```
cc -onob nob.c && ./nob
```

# And run: 
```
./build/main
```
