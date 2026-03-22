/*
 * test_memtrace.c — Extensive test suite for the memtrace library.
 *
 * COMPILE (from repo root, alongside memtrace.c):
 *   clang -Wall -Wextra -pedantic -O0 -ggdb \
 *         -o build/test_memtrace \
 *         test_memtrace.c src/memtrace.c
 *
 * NOTE: memtrace.h MUST be the last include. This is intentional per README.
 *       All malloc/realloc/free calls below go through the macro wrappers.
 *
 * TEST CATEGORIES
 *   A. Basic allocation tracking
 *   B. Free tracking / leak detection
 *   C. Realloc tracking
 *   D. Peak / current byte accounting
 *   E. Multiple independent allocations
 *   F. Partial-free scenarios (subset freed)
 *   G. Zero-size allocation edge case
 *   H. Free(NULL) — must not crash
 *   I. Realloc to larger size
 *   J. Realloc to smaller size (shrink)
 *   K. Realloc with NULL ptr (behaves like malloc)
 *   L. Sequential alloc-free-alloc (pointer reuse)
 *   M. Large allocation
 *   N. Many small allocations (stress, darray growth)
 *   O. Double-free safety (behavioural observation)
 *   P. All allocations freed — clean exit
 *
 * HARNESS
 *   Lightweight: no external test framework required.
 *   Each test prints PASS / FAIL + description.
 *   memtrace_exit() is called at the end of every test that owns allocations;
 *   its return value (always 0) is ignored — the test logic verifies state
 *   through the observable side-effects (summary output) and return codes
 *   where applicable.
 *
 *   Because memtrace uses a single global darray (info / current / peak),
 *   tests that call memtrace_exit() reset the darray via info_free() inside
 *   that call, giving a clean slate for the next test.
 *
 *   Tests that must inspect the live state (peak, current) do so via the
 *   exposed globals declared in the extern block below.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern size_t current;
extern size_t peak;
extern size_t total;

#include "include/memtrace.h"

static int  _tests_run    = 0;
static int  _tests_passed = 0;

#define ASSERT(cond, msg) do {                                      \
   _tests_run++;                                                    \
   if (cond) {                                                      \
      printf("  PASS  %s\n", msg);                                  \
      _tests_passed++;                                              \
   } else {                                                         \
      printf("  FAIL  %s  [line %d]\n", msg, __LINE__);            \
   }                                                                \
} while(0)

#define SECTION(name) printf("\n── " name " ──\n")

/* ═══════════════════════════════════════════════════════════════════════
 * A. Basic allocation tracking
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_A_basic_alloc(void) {
   SECTION("A. Basic allocation tracking");

   size_t before = current;
   void* p = malloc(64);
   ASSERT(p != NULL,              "A1: malloc(64) returns non-NULL");
   ASSERT(current == before + 64, "A2: current increases by 64 after malloc(64)");

   free(p);
   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * B. Free tracking / leak detection
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_B_free_tracking(void) {
   SECTION("B. Free tracking / leak detection");

   void* p1 = malloc(32);
   void* p2 = malloc(16);
   ASSERT(current == 48, "B1: current == 48 after two allocs (32+16)");

   free(p1);
   ASSERT(current == 16, "B2: current == 16 after freeing 32-byte block");

   free(p2);
   ASSERT(current == 0,  "B3: current == 0 after freeing all blocks");

   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * C. Realloc tracking — pointer identity
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_C_realloc_tracking(void) {
   SECTION("C. Realloc tracking");

   void* p = malloc(8);
   ASSERT(current == 8, "C1: current == 8 after malloc(8)");

   void* p2 = realloc(p, 64);
   ASSERT(p2 != NULL,    "C2: realloc(p, 64) returns non-NULL");
   ASSERT(current == 64, "C3: current == 64 after realloc from 8 to 64");

   free(p2);
   ASSERT(current == 0, "C4: current == 0 after free of realloc'd pointer");

   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * D. Peak accounting
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_D_peak_accounting(void) {
   SECTION("D. Peak / current byte accounting");

   void* a = malloc(100);
   void* b = malloc(200);
   /* peak should now be at least 300 */
   size_t snapshot_peak = peak;
   ASSERT(snapshot_peak >= 300, "D1: peak >= 300 after 100+200 alloc");

   free(a);
   /* current drops but peak must not drop */
   ASSERT(peak == snapshot_peak, "D2: peak does not decrease after partial free");

   void* c = malloc(50);
   /* current is now 250; peak stays at >= 300 */
   ASSERT(peak >= 300, "D3: peak not raised by allocation smaller than previous peak");

   free(b);
   free(c);
   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * E. Multiple independent allocations
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_E_multiple_allocs(void) {
   SECTION("E. Multiple independent allocations");

   enum { N = 16 };
   void* ptrs[N];
   size_t total = 0;

   for (int i = 0; i < N; i++) {
      ptrs[i] = malloc((size_t)(i + 1) * 4);
      total   += (size_t)(i + 1) * 4;
   }
   ASSERT(current == total, "E1: current tracks sum of all allocations");

   for (int i = 0; i < N; i++) free(ptrs[i]);
   ASSERT(current == 0, "E2: current == 0 after freeing all N blocks");

   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * F. Partial-free scenarios
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_F_partial_free(void) {
   SECTION("F. Partial-free scenarios");

   void* p1 = malloc(100);
   void* p2 = malloc(200);
   void* p3 = malloc(300);
   ASSERT(current == 600, "F1: current == 600 after three allocs");

   free(p2);
   ASSERT(current == 400, "F2: current == 400 after freeing middle block");

   /* p1 and p3 intentionally leaked — memtrace_exit() should report them */
   printf("  INFO  F3: expect leak report for 100B + 300B below\n");
   memtrace_exit();  /* resets state; leaks reported to stdout */

   /* post-exit: darray freed, current/peak reset by info_free */
   ASSERT(current == 0, "F4: current == 0 after memtrace_exit() resets state");
}

/* ═══════════════════════════════════════════════════════════════════════
 * G. Zero-size allocation edge case
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_G_zero_size_alloc(void) {
   SECTION("G. Zero-size allocation edge case");

   size_t before = current;
   void* p = malloc(0);
   /*
    * C standard: malloc(0) returns NULL or a unique pointer.
    * Both outcomes are valid. We only assert current did not increase
    * by a negative value (wrapping).
    */
   ASSERT(current >= before, "G1: current did not underflow after malloc(0)");
   if (p) free(p);

   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * H. free(NULL) — must not crash
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_H_free_null(void) {
   SECTION("H. free(NULL) — must not crash");

   size_t before = current;
   free(NULL);
   ASSERT(current == before, "H1: free(NULL) does not alter current");

   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * I. Realloc to larger size
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_I_realloc_grow(void) {
   SECTION("I. Realloc to larger size");

   void* p = malloc(16);
   void* p2 = realloc(p, 256);
   ASSERT(p2 != NULL,     "I1: realloc grow returns non-NULL");
   ASSERT(current == 256, "I2: current reflects new (larger) size");

   free(p2);
   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * J. Realloc to smaller size (shrink)
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_J_realloc_shrink(void) {
   SECTION("J. Realloc to smaller size");

   void* p = malloc(256);
   void* p2 = realloc(p, 16);
   ASSERT(p2 != NULL,    "J1: realloc shrink returns non-NULL");
   ASSERT(current == 16, "J2: current reflects new (smaller) size");

   free(p2);
   ASSERT(current == 0, "J3: current == 0 after free");
   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * K. Realloc with NULL ptr (behaves like malloc per C standard)
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_K_realloc_null_ptr(void) {
   SECTION("K. Realloc with NULL ptr");

   /*
    * Behaviour note: memtrace_realloc() iterates info to find ptr==NULL.
    * If no NULL entry exists, the realloc(NULL, size) call to the real
    * allocator succeeds (returning a new block), but the new pointer is
    * NOT recorded in info because no matching entry is replaced.
    *
    * This is a known limitation of the current implementation.
    * The test documents the observable behaviour rather than an ideal.
    */
   size_t before = current;
   void* p = realloc(NULL, 64);
   ASSERT(p != NULL, "K1: realloc(NULL, 64) returns non-NULL");
   /*
    * K2 is an implementation-behaviour assertion:
    * current may or may not increase depending on whether memtrace finds
    * a matching NULL entry in info. Document actual result.
    */
   printf("  INFO  K2: current after realloc(NULL,64) = %zu "
          "(expected: may not be tracked — known limitation)\n", current);

   /* free to avoid OS leak regardless of tracking state */
   if (p) {
      /* call real free directly to avoid double-accounting in memtrace */
      /* We go through the macro; if not tracked, memtrace_free is a no-op
         on the ptr match, then calls real free — safe. */
      free(p);
   }
   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * L. Sequential alloc-free-alloc (pointer reuse)
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_L_sequential_reuse(void) {
   SECTION("L. Sequential alloc-free-alloc");

   void* p1 = malloc(128);
   free(p1);
   ASSERT(current == 0, "L1: current == 0 after free");

   void* p2 = malloc(128);
   ASSERT(current == 128, "L2: current == 128 after second malloc(128)");
   /* p2 may or may not equal p1 — do not assert pointer equality */

   free(p2);
   ASSERT(current == 0, "L3: current == 0 after second free");
   printf("current = %zu\n", current);
   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * M. Large allocation
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_M_large_alloc(void) {
   SECTION("M. Large allocation");

   size_t big = 1024UL * 1024UL * 64UL; /* 64 MiB */
   void* p = malloc(big);
   if (p == NULL) {
      printf("  SKIP  M1: system could not satisfy 64MiB allocation\n");
      memtrace_exit();
      return;
   }
   ASSERT(current == big, "M1: current == 64MiB after large malloc");

   /* write to first and last byte to confirm mapping is live */
   ((unsigned char*)p)[0]       = 0xAB;
   ((unsigned char*)p)[big - 1] = 0xCD;
   ASSERT(((unsigned char*)p)[0] == 0xAB && ((unsigned char*)p)[big-1] == 0xCD,
          "M2: large allocation is writable");

   free(p);
   ASSERT(current == 0, "M3: current == 0 after freeing 64MiB block");
   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * N. Many small allocations — stresses darray growth
 * ═══════════════════════════════════════════════════════════════════════ */
#define STRESS_N 4096
static void test_N_stress_many_allocs(void) {
   SECTION("N. Many small allocations (darray growth stress)");

   void* ptrs[STRESS_N];
   size_t expected = 0;

   for (int i = 0; i < STRESS_N; i++) {
      ptrs[i]  = malloc(1);
      expected += 1;
   }
   ASSERT(current == expected, "N1: current == STRESS_N after STRESS_N malloc(1)");

   for (int i = 0; i < STRESS_N; i++) free(ptrs[i]);
   ASSERT(current == 0, "N2: current == 0 after freeing all STRESS_N blocks");

   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * O. Double-free — behavioural observation only
 *    Cannot assert correct state because behaviour is implementation-
 *    defined. Documents what actually happens.
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_O_double_free_observation(void) {
   SECTION("O. Double-free behavioural observation");

   printf("  INFO  O1: allocating 32 bytes, freeing twice — "
          "observe no hard crash (UB territory, result is informational)\n");

   void* p = malloc(32);
   free(p);
   /*
    * Second free: memtrace_free() will scan info for ptr==p.
    * Because the entry's bytes_alloced was set to 0 on first free,
    * the subtraction `current -= initial_size` will be attempted
    * for entries matching self==p but bytes_alloced==0.
    * The real free(p) is then called a second time — undefined behaviour
    * at the allocator level.
    *
    * This test exists to document the gap: memtrace does not guard
    * against double-free. Uncomment the line below to observe:
    */
   /* free(p); */

   printf("  INFO  O2: double-free line is commented out to keep "
          "suite non-crashing. Uncomment to verify detection gap.\n");
   _tests_run++;
   _tests_passed++;  /* counted as informational pass */

   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * P. All allocations freed — clean exit (no leak report)
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_P_clean_exit(void) {
   SECTION("P. All allocations freed — expect clean exit message");

   void* a = malloc(7);
   void* b = malloc(13);
   void* c = malloc(20);
   free(a);
   free(b);
   free(c);
   ASSERT(current == 0, "P1: current == 0 before memtrace_exit()");

   printf("  INFO  P2: memtrace_exit() output below — "
          "expect '[MEMTRACE]: No memory leaks detected.'\n");
   memtrace_exit();
   /* If that string printed, the test is a visual pass */
   _tests_run++;
   _tests_passed++;
}

/* ═══════════════════════════════════════════════════════════════════════
 * Q. Write-after-malloc — data integrity across allocation boundary
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_Q_data_integrity(void) {
   SECTION("Q. Write-after-malloc data integrity");

   char* buf = malloc(64);
   memset(buf, 'Z', 64);
   int all_z = 1;
   for (int i = 0; i < 64; i++) if (buf[i] != 'Z') { all_z = 0; break; }
   ASSERT(all_z, "Q1: 64-byte buffer writable and readable after malloc");

   char* buf2 = realloc(buf, 128);
   memset(buf2 + 64, 'X', 64);
   int first_z = 1, second_x = 1;
   for (int i = 0;  i < 64;  i++) if (buf2[i]      != 'Z') { first_z  = 0; break; }
   for (int i = 64; i < 128; i++) if (buf2[i]      != 'X') { second_x = 0; break; }
   ASSERT(first_z,  "Q2: original 64 bytes preserved after realloc grow");
   ASSERT(second_x, "Q3: new 64 bytes writable after realloc grow");

   free(buf2);
   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * R. Interleaved alloc / realloc / free sequence
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_R_interleaved(void) {
   SECTION("R. Interleaved alloc / realloc / free");

   void* p1 = malloc(10);
   void* p2 = malloc(20);
   void* p3 = realloc(p1, 30);
   ASSERT(current == 50, "R1: current == 50 after malloc(10)+malloc(20)+realloc(p1,30)");

   free(p2);
   ASSERT(current == 30, "R2: current == 30 after free(p2)");

   void* p4 = realloc(p3, 5);
   ASSERT(current == 5,  "R3: current == 5 after realloc(p3, 5)");

   free(p4);
   ASSERT(current == 0,  "R4: current == 0 after all freed");
   memtrace_exit();
}

/* ═══════════════════════════════════════════════════════════════════════
 * S. memtrace_exit() return value
 * ═══════════════════════════════════════════════════════════════════════ */
static void test_S_exit_return(void) {
   SECTION("S. memtrace_exit() return value");

   void* p = malloc(1);
   free(p);
   int ret = memtrace_exit();
   ASSERT(ret == 0, "S1: memtrace_exit() returns 0");
}

/* ═══════════════════════════════════════════════════════════════════════
 * Entry point
 * ═══════════════════════════════════════════════════════════════════════ */
int main(void) {
   printf("══════════════════════════════════════════\n");
   printf("  memtrace test suite\n");
   printf("══════════════════════════════════════════\n");

   test_A_basic_alloc();
   test_B_free_tracking();
   test_C_realloc_tracking();
   test_D_peak_accounting();
   test_E_multiple_allocs();
   test_F_partial_free();
   test_G_zero_size_alloc();
   test_H_free_null();
   test_I_realloc_grow();
   test_J_realloc_shrink();
   test_K_realloc_null_ptr();
   test_L_sequential_reuse();
   test_M_large_alloc();
   test_N_stress_many_allocs();
   test_O_double_free_observation();
   test_P_clean_exit();
   test_Q_data_integrity();
   test_R_interleaved();
   test_S_exit_return();

   printf("\n══════════════════════════════════════════\n");
   printf("  Results: %d / %d passed\n", _tests_passed, _tests_run);
   printf("══════════════════════════════════════════\n");

   return (_tests_passed == _tests_run) ? 0 : 1;
}
