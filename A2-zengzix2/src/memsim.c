/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC369H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors: Mario Badr
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2024 Mario Badr
 */
#include "csc369_kmalloc.h"
#include "csc369_logger.h"
#include "csc369_policy.h"
#include "csc369_simulation.h"

#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

// Make simulation global, so we can destroy the swap file on a fatal error
CSC369_Simulation* simulation = NULL;

/* The policies array gives us a mapping between the name of an eviction
 * algorithm as given in a command line argument, and the function to
 * call to select the victim page.
 *
 * The list of REPLACEMENT_ALGORITHMS is found in csc369_policy.h
 * We use the C preprocessor stringizing and concatenation operations to
 * create a template for the algorithm function structure.
 * See https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
 * and https://gcc.gnu.org/onlinedocs/cpp/Concatenation.html
 */
static CSC369_Policy CSC369_POLICIES[] = {
#define CSC369_POLICY_DECL(name)                                               \
  { #name,                                                                     \
    CSC369_Policy##name##Init,                                                 \
    CSC369_Policy##name##Cleanup,                                              \
    CSC369_Policy##name##OnRef,                                                \
    CSC369_Policy##name##Evict },
  CSC369_POLICY_NAMES
};

static size_t CSC369_POLICIES_COUNT =
  sizeof(CSC369_POLICIES) / sizeof(CSC369_POLICIES[0]);

// Returns current time in seconds since the Epoch is a floating point number
static inline double
get_time()
{
  struct timespec t;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
  long total = t.tv_sec + t.tv_nsec;
  return ((double)total) / 1000000000.0;
}

// To remove the swapfile on failure
static void
InstallFatalHandlers();

static CSC369_Policy*
GetReplacementAlgorithm(char const* name)
{
  for (size_t i = 0; i < CSC369_POLICIES_COUNT; i++) {
    if (strcmp(CSC369_POLICIES[i].name, name) == 0) {
      CSC369_LOG_INFO("Replacement: %s.", name);
      return &CSC369_POLICIES[i];
    }
  }

  CSC369_LOG_ERROR("A replacement algorithm for %s was not found.", name);
  return NULL;
}

static void
usage(char const* prog)
{
  fprintf(stderr,
          "USAGE: %s -f tracefile "
          "-m memorysize -s swapsize -a algorithm [-v num -p]\n",
          prog);
  fprintf(stderr, "\t-f tracefile  - path to trace file to simulate\n");
  fprintf(stderr, "\t-m memorysize - number of physical memory frames\n");
  fprintf(stderr, "\t-s swapsize   - number of frames in swapfile\n");
  fprintf(stderr, "\t-a algorithm  - replacement algorithm to use, one of:\n");
  for (size_t i = 0; i < CSC369_POLICIES_COUNT; ++i) {
    fprintf(stderr, "\t\t%s\n", CSC369_POLICIES[i].name);
  }
}

static CSC369_Simulation*
CreateSimulationFromArgs(int argc, char** argv)
{
  size_t memsize = 0;
  size_t swapsize = 0;
  char const* trace_file = NULL;
  char const* replacement_alg = NULL;
  int opt;

  while ((opt = getopt(argc, argv, "f:m:a:s:h")) != -1) {
    switch (opt) {
      case 'f':
        trace_file = optarg;
        CSC369_LOG_INFO("Trace file: %s.", trace_file);
        break;
      case 'm':
        memsize = strtoul(optarg, NULL, 10);
        CSC369_LOG_INFO("Number of frames (physical memory): %zu.", memsize);
        break;
      case 'a':
        replacement_alg = optarg;
        break;
      case 's':
        swapsize = strtoul(optarg, NULL, 10);
        CSC369_LOG_INFO("Number of frames (swap): %zu.", swapsize);
        break;
      case 'h':
      default:
        usage(argv[0]);
        return NULL;
    }
  }

  if (!trace_file || !memsize || !swapsize || !replacement_alg) {
    usage(argv[0]);
    return NULL;
  }

  return CSC369_SimulationCreate(
    trace_file, memsize, swapsize, GetReplacementAlgorithm(replacement_alg));
}

int
main(int argc, char* argv[])
{
  CSC369_MallocInit(true);
  InstallFatalHandlers();

  // Get initial memory use after initializing main simulation data structures.
  long start_mallocs = CSC369_MallocCurrentCount();
  long start_bytes = CSC369_MallocCurrentBytes();

  double start_time = get_time();
  simulation = CreateSimulationFromArgs(argc, argv);
  if (simulation == NULL) {
    return 0;
  }
  CSC369_SimulationRun(simulation);
  double end_time = get_time();

  // Get final memory use.
  long bytes_used = CSC369_MallocCurrentBytes() - start_bytes;
  CSC369_LOG_INFO("Time to run simulation: %f.", end_time - start_time);
  CSC369_LOG_INFO("Memory used by simulation: %ld bytes.", bytes_used);

  CSC369_SimulationDestroy(simulation);

  // Check for memory leaks via kmalloc
  if (CSC369_MallocIsLeakFree(start_mallocs, start_bytes)) {
    CSC369_LOG_INFO("No memory leaks with kmalloc detected.");
  } else {
    long bytes_leaked = CSC369_MallocCurrentBytes();
    long unfreed_mallocs = CSC369_MallocCurrentCount();
    CSC369_LOG_INFO("Detected %lu bytes leaked from %lu un-freed mallocs.",
                    bytes_leaked,
                    unfreed_mallocs);
  }

  return 0;
}

/*********** TRY TO CATCH SIGNALS FOR ERROR REPORTING ***************/

static void
HandleFatalSignal(int signum, siginfo_t* info, void* context)
{
  UNUSED(info);
  UNUSED(context);

  CSC369_LOG_ERROR("A fatal error occurred. Attempt to clean up swap file.");
  CSC369_SimulationDestroySwap(simulation);
  exit(signum);
}

static void
InstallFatalHandlers(void)
{
  struct sigaction sig_action;
  struct sigaction old_action;

  memset(&sig_action, 0, sizeof(sig_action));
  sig_action.sa_sigaction = HandleFatalSignal;
  sig_action.sa_flags = SA_RESTART | SA_SIGINFO;
  sigemptyset(&sig_action.sa_mask);

  sigaction(SIGSEGV, &sig_action, &old_action);
  sigaction(SIGABRT, &sig_action, &old_action);
  sigaction(SIGTRAP, &sig_action, &old_action);
  sigaction(SIGILL, &sig_action, &old_action);
  sigaction(SIGFPE, &sig_action, &old_action);
}
