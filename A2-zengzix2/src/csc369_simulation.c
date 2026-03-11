/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC369H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2019, 2021 Karen Reid
 * Copyright (c) 2023, Angela Brown, Kuei Sun
 * Copyright (c) 2024 Mario Badr
 */
#include "csc369_simulation.h"

#include "csc369_coremap.h"
#include "csc369_logger.h"
#include "csc369_mm.h"
#include "csc369_swap.h"
#include "csc369_trace.h"

#include <assert.h>
#include <stdlib.h>

/* Global variables. */
size_t CSC369_GLOBAL_MEMSIZE = 0;
CSC369_MMStats CSC369_GLOBAL_STATS;

/* Global function pointers. */
void (*CSC369_OnReference)(CSC369_Frame*) = NULL;
uint64_t (*CSC369_Evict)(CSC369_Frame*) = NULL;

typedef struct csc369_simulation_t
{
  CSC369_Trace* trace;

  CSC369_Policy* policy;
  CSC369_Swap* swap;
  CSC369_Coremap* coremap;
  uint8_t* physmem;
  CSC369_MM* mm;
} CSC369_Simulation;

CSC369_Simulation*
CSC369_SimulationCreate(char const* trace_file,
                        size_t frame_count,
                        size_t swap_size,
                        CSC369_Policy* policy)
{
  assert(trace_file != NULL);
  assert(policy != NULL);

  CSC369_Simulation* simulation = calloc(1, sizeof(CSC369_Simulation));
  if (simulation == NULL) {
    CSC369_LOG_ERRNO("calloc");
    return NULL;
  }

  simulation->trace = CSC369_TraceCreate(trace_file);
  if (simulation->trace == NULL) {
    CSC369_SimulationDestroy(simulation);
    return NULL;
  }

  simulation->physmem = calloc(frame_count, CSC369_SIMULATED_FRAME_SIZE);
  if (simulation->physmem == NULL) {
    CSC369_LOG_ERRNO("calloc");
    CSC369_SimulationDestroy(simulation);
    return NULL;
  }

  simulation->swap = CSC369_SwapCreate(swap_size, simulation->physmem);
  if (simulation->swap == NULL) {
    CSC369_SimulationDestroy(simulation);
    return NULL;
  }

  simulation->coremap = CSC369_CoremapCreate(simulation->swap, frame_count);
  if (simulation->coremap == NULL) {
    CSC369_SimulationDestroy(simulation);
    return NULL;
  }

  simulation->mm =
    CSC369_MMCreate(simulation->coremap, simulation->swap, simulation->physmem);
  if (simulation->mm == NULL) {
    CSC369_SimulationDestroy(simulation);
    return NULL;
  }

  // Save global variable values
  CSC369_GLOBAL_MEMSIZE = frame_count;
  // Save the policy to call its cleanup function later
  simulation->policy = policy;
  // Initialize the policy
  policy->CSC369_PolicyInit();
  // Save pointers to the functions in globally accessible variables, used in
  // coremap
  CSC369_OnReference = policy->CSC369_PolicyOnRef;
  CSC369_Evict = policy->CSC369_PolicyEvict;

  return simulation;
}

void
CSC369_SimulationDestroy(CSC369_Simulation* simulation)
{
  assert(simulation != NULL);

  if (simulation->trace != NULL) {
    CSC369_TraceDestroy(simulation->trace);
  }

  if (simulation->physmem != NULL) {
    free(simulation->physmem);
  }

  if (simulation->coremap != NULL) {
    CSC369_CoremapDestroy(simulation->coremap);
  }

  if (simulation->swap != NULL) {
    CSC369_SwapDestroy(simulation->swap, true);
  }

  if (simulation->mm != NULL) {
    CSC369_MMDestroy(simulation->mm);
  }

  if (simulation->policy != NULL) {
    simulation->policy->CSC369_PolicyCleanup();
  }

  free(simulation);
}

void
CSC369_SimulationDestroySwap(CSC369_Simulation* simulation)
{
  if (simulation->swap != NULL) {
    CSC369_SwapDestroy(simulation->swap, false);
  }
}

/**
 * "Execute" a "memory instruction", which is modelled as an access.
 *
 * @param simulation A pointer to the simualtion.
 * @param access The memory access.
 */
static void
ExecuteMemInstruction(CSC369_Simulation* simulation, CSC369_MemAccess access)
{
  // Ask the MM to do the translation (and anything else that's needed) to get
  // the frame number
  uint64_t const frame_number = CSC369_MMGetFrameNumber(simulation->mm, access);

  // Call the replacement algorithm's OnRef function
  CSC369_OnReference(CSC369_CoremapGetFrame(simulation->coremap, frame_number));

  // Get pointer to (simulated) physical memory
  off_t offset = (off_t)access.virtual_address % CSC369_PAGE_SIZE;
  uint8_t* phys_addr =
    &simulation->physmem[frame_number * CSC369_SIMULATED_FRAME_SIZE] + offset;

  if (access.type == CSC369_ACCESS_TYPE_STORE ||
      access.type == CSC369_ACCESS_TYPE_MODIFY) {
    // This is a write access to the page, so update the value in simulated
    // memory.
    *phys_addr = access.value;
  } else {
    // This is a read access to the page, so check that the value read is
    // correct.
    uint8_t actual_value = *phys_addr;
    if (actual_value != access.value) {
      CSC369_LOG_ERROR("Data at address 0x%012lx (frame %lu) was %hhu, but "
                       "should have been %hhu.",
                       access.virtual_address,
                       frame_number,
                       actual_value,
                       access.value);
    }
  }
}

static void
LogStats(CSC369_MMStats stats)
{
  CSC369_LOG_INFO("Total reference count: %u", stats.ref_count);
  CSC369_LOG_INFO("Hit count: %u", stats.hit_count);
  CSC369_LOG_INFO("Miss count: %u", stats.miss_count);
  CSC369_LOG_INFO("Clean evictions: %u", stats.evict_clean_count);
  CSC369_LOG_INFO("Dirty evictions: %u", stats.evict_dirty_count);
  CSC369_LOG_INFO("Hit rate: %.4f",
                  ((double)stats.hit_count / stats.ref_count) * 100.0);
  CSC369_LOG_INFO("Miss rate: %.4f",
                  ((double)stats.miss_count / stats.ref_count) * 100.0);
}

void
CSC369_SimulationRun(CSC369_Simulation* simulation)
{
  assert(simulation != NULL);

  CSC369_LOG_TRACE("Starting simulation.");
  CSC369_MemAccess mem_access;
  while (CSC369_TraceRead(simulation->trace, &mem_access) == 0) {
    ExecuteMemInstruction(simulation, mem_access);
  }

  LogStats(CSC369_GLOBAL_STATS);
  CSC369_LOG_TRACE("Simulation complete.");
}
