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
#include "csc369_coremap.h"

#include "csc369_def.h"
#include "csc369_logger.h"
#include "csc369_mm.h"
#include "csc369_swap.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct csc369_coremap_t
{
  CSC369_Frame* frames;
  size_t frame_count;
  CSC369_Swap* swap;
} CSC369_Coremap;

CSC369_Coremap*
CSC369_CoremapCreate(CSC369_Swap* swap, size_t num_frames)
{
  CSC369_Coremap* coremap = calloc(1, sizeof(CSC369_Coremap));
  if (coremap == NULL) {
    CSC369_LOG_ERRNO("calloc");
    return NULL;
  }

  coremap->frames = calloc(num_frames, sizeof(CSC369_Frame));
  if (coremap->frames == NULL) {
    CSC369_LOG_ERRNO("calloc");
    CSC369_CoremapDestroy(coremap);
    return NULL;
  }

  coremap->frame_count = num_frames;
  for (size_t i = 0; i < num_frames; i++) {
    coremap->frames[i].free = true;
  }

  coremap->swap = swap;

  return coremap;
}

void
CSC369_CoremapDestroy(CSC369_Coremap* coremap)
{
  assert(coremap != NULL);

  if (coremap->frames != NULL) {
    free(coremap->frames);
  }

  free(coremap);
}

CSC369_Frame*
CSC369_CoremapGetFrame(CSC369_Coremap* coremap, uint64_t frame_number)
{
  assert(coremap != NULL);
  assert(frame_number < CSC369_GLOBAL_MEMSIZE);

  return &coremap->frames[frame_number];
}

static bool
FindFreeFrame(CSC369_Coremap* coremap, uint64_t* frame_number)
{
  assert(coremap != NULL);

  for (size_t i = 0; i < coremap->frame_count; i++) {
    if (coremap->frames[i].free) {
      *frame_number = i;
      return true;
    }
  }

  return false;
}

void
CSC369_CoremapClearFrame(uint8_t* physmem, uint64_t frame_number)
{
  assert(physmem != NULL);
  assert(frame_number < CSC369_GLOBAL_MEMSIZE);

  // Update values in memory
  uint8_t* mem_ptr = physmem + (frame_number * CSC369_SIMULATED_FRAME_SIZE);
  memset(mem_ptr, 0, CSC369_SIMULATED_FRAME_SIZE); // zero-fill the frame
}

uint64_t
CSC369_CoremapAllocateFrame(CSC369_Coremap* coremap, csc369_pte_t* pte)
{
  assert(coremap != NULL);
  assert(pte != NULL);

  uint64_t frame_number;
  int found = FindFreeFrame(coremap, &frame_number);

  if (!found) {
    // Call replacement algorithm to find a victim
    frame_number = CSC369_Evict(coremap->frames);

    csc369_pte_t* victim = coremap->frames[frame_number].pte;
    assert(victim != NULL);

    // All frames were in use, so the victim frame holds some page.
    // Write victim to swap, if needed, and update the page table.
    CSC369_MMHandleEviction(coremap->swap, victim);
  }

  // Update coremap accounting
  coremap->frames[frame_number].free = false;
  coremap->frames[frame_number].pte = pte;

  return frame_number;
}
