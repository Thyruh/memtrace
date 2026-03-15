#define NOB_IMPLEMENTATION

#include "nob.h"

#define CXX "gcc"
#define CFLAGS "-Wall", "-Wextra", "-pedantic", "-O3"
#define CDEBUGFLAGS "-ggdb", "-O0"
#define DESTINATION "build/"
#define SRC "src/"

int main(int argc, char** argv) {
   NOB_GO_REBUILD_URSELF(argc, argv);
   Nob_Cmd cmd = {0};
   nob_cmd_append(&cmd, CXX, "-o", DESTINATION"main", SRC"main.c", SRC"memtrace.c", CFLAGS);
   if (!nob_cmd_run(&cmd)) return 1;
   return 0;
}

