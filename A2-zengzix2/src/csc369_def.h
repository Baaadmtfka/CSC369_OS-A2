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
#ifndef CSC369H1_MEMORY_DEF_H
#define CSC369H1_MEMORY_DEF_H

#include <stddef.h>
#include <stdint.h>

/**
 * Virtual addresses are 48 bits.
 */
#define CSC369_VADDR_BITS 48

/**
 * The least significant 12 bits are for the offset.
 */
#define CSC369_VADDR_OFFSET_BITS 12

/**
 * The most significant 38 bits are for the page number.
 */
#define CSC369_VADDR_PAGE_NUMBER_BITS 36

/**
 * The size of a page (and frame), in bytes.
 */
#define CSC369_PAGE_SIZE (1 << CSC369_VADDR_OFFSET_BITS)

/**
 * The size of a simulated frame, in bytes.
 */
#define CSC369_SIMULATED_FRAME_SIZE 16

/**
 * Different types of memory accesses.
 */
typedef enum
{
  CSC369_ACCESS_TYPE_STORE,
  CSC369_ACCESS_TYPE_MODIFY,
  CSC369_ACCESS_TYPE_LOAD,
  CSC369_ACCESS_TYPE_INSTRUCTION_FETCH,
} CSC369_AccessType;

typedef struct csc369_mem_access_t
{
  /**
   * The type of memory access.
   */
  CSC369_AccessType type;

  /**
   * The virtual address being accessed.
   */
  uint64_t virtual_address;

  /**
   * The value to, depending on type, either:
   *    1. Write to the address.
   *    2. Check the contents of the address match.
   */
  uint8_t value;
} CSC369_MemAccess;

/** Number of frames of physical memory */
extern size_t CSC369_GLOBAL_MEMSIZE;

/** A pointer to the replacement policy's OnReference function. */
typedef struct csc369_frame_t CSC369_Frame; // forward declaration
extern void (*CSC369_OnReference)(CSC369_Frame*);

/** A pointer to the replacement algorithm's Evict function. */
extern uint64_t (*CSC369_Evict)(CSC369_Frame* all_frames);

/**
 * A bundle of CSC369_GLOBAL_STATS.
 */
typedef struct
{
  /**
   * Total number of memory accesses.
   */
  uint32_t ref_count;

  /**
   * Total number of hits.
   */
  uint32_t hit_count;

  /**
   * Total number of major page faults.
   */
  uint32_t miss_count;

  /**
   * Total number of clean evictions.
   */
  uint32_t evict_clean_count;

  /**
   * Total number of dirty evictions.
   */
  uint32_t evict_dirty_count;
} CSC369_MMStats;

extern CSC369_MMStats CSC369_GLOBAL_STATS;

#endif // CSC369H1_MEMORY_DEF_H
