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
#ifndef CSC369H1_MEMORY_COREMAP_H
#define CSC369H1_MEMORY_COREMAP_H

#include "csc369_policy.h"
#include "csc369_pte.h"
#include "list.h"

#include <stddef.h>
#include <stdint.h>

typedef struct csc369_swap_t CSC369_Swap; // forward declaration

/**
 * The coremap manages physical frames.
 *
 * Each frame has a frame number, which is a value between [0,
 * CSC369_GLOBAL_MEMSIZE). Frame numbers outside of this range are invalid.
 */
typedef struct csc369_coremap_t CSC369_Coremap;

/**
 * A physical frame.
 */
typedef struct csc369_frame_t
{
  /**
   * true if the frame is free, false otherwise.
   */
  bool free;

  /**
   * A pointer to the pte of the page stored in this frame.
   */
  csc369_pte_t* pte;

  /**
   * Useful for some replacement algorithms.
   */
  list_entry framelist_entry;
} CSC369_Frame;

/**
 * Create a coremap.
 *
 * @param swap A pointer to the swap space to handle evictions.
 * @param num_frames The size of memory.
 *
 * @return A (newly-allocated) coremap, or NULL on error.
 */
CSC369_Coremap*
CSC369_CoremapCreate(CSC369_Swap* swap, size_t num_frames);

/**
 * Destroy the coremap and free up any resources it holds.
 *
 * @param coremap A pointer to the coremap.
 *
 * @pre coremap is not NULL.
 */
void
CSC369_CoremapDestroy(CSC369_Coremap* coremap);

/**
 * Get the frame associated with the frame number.
 *
 * @param coremap A pointer to the coremap.
 * @param frame_number The frame number.
 *
 * @return A pointer to the frame at this frame number.
 *
 * @pre coremap is not NULL. The frame_number is valid.
 */
CSC369_Frame*
CSC369_CoremapGetFrame(CSC369_Coremap* coremap, uint64_t frame_number);

/**
 * Set all the bytes in the frame at physmem to 0.
 *
 * Should beused when a frame is first allocated for some virtual address. Just
 * like in a real OS, we fill the frame with zeros to prevent leaking
 * information across pages.
 *
 * @param physmem A pointer to physical memory.
 * @param frame_number The frame number.
 *
 * @pre physmem is not NULL. The frame_number is valid.
 */
void
CSC369_CoremapClearFrame(uint8_t* physmem, uint64_t frame_number);

/**
 * Allocate a frame so that it can be mapped to a page. If no free frames are
 * available, this function will evict a non-free frame.
 *
 * Eviction involves using the replacement algorithm to find a victim, and then
 * asking the MM to handle the eviction.
 *
 * @param coremap A pointer to the coremap.
 * @param pte A pointer to the PTE being mapped to this frame.
 *
 * @return the frame number.
 *
 * @pre coremap and pte are not NULL.
 */
uint64_t
CSC369_CoremapAllocateFrame(CSC369_Coremap* coremap, csc369_pte_t* pte);

#endif // CSC369H1_MEMORY_COREMAP_H
