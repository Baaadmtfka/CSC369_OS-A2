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
#ifndef CSC369H1_MEMORY_MM_H
#define CSC369H1_MEMORY_MM_H

#include "csc369_def.h"

// forward declarations
typedef struct csc369_pte_t csc369_pte_t;
typedef struct csc369_page_table_t CSC369_PageTable;
typedef struct csc369_coremap_t CSC369_Coremap;
typedef struct csc369_swap_t CSC369_Swap;

/**
 * The memory-management part of a mm.
 */
typedef struct csc369_mm_t CSC369_MM;

/**
 * Create a mm.
 *
 * @param num_frames The size of memory.
 *
 * @return A (newly-allocated) mm, or NULL on error.
 */
CSC369_MM*
CSC369_MMCreate(CSC369_Coremap* coremap, CSC369_Swap* swap, uint8_t* physmem);

/**
 * Destroy the mm and free up any resources it holds.
 *
 * @param mm A pointer to the mm.
 *
 * @pre mm is not NULL.
 */
void
CSC369_MMDestroy(CSC369_MM* mm);

/**
 * Find the frame number associated with this memory access using the page
 * table.
 *
 * If the page table entry is invalid and not on swap, then this is the first
 * reference to the page and a (simulated) physical frame should be allocated
 * and initialized to all zeros.
 *
 * If the page table entry is invalid and on swap, then a (simulated) physical
 * frame should be allocated and filled by reading the page data from swap.
 *
 * Make sure to update page table entry status information:
 *  - the page table entry should be marked valid
 *  - if the type of access is a write ('S'tore or 'M'odify),
 *    the page table entry should be marked dirty
 *  - a page should be marked dirty on the first reference to the page,
 *    even if the type of access is a read ('L'oad or 'I'nstruction type).
 *  - DO NOT UPDATE the page table entry 'referenced' information. That
 *    should be done by the replacement algorithm functions.
 *
 * When you have a valid page table entry, return the page frame number
 * that holds the requested virtual page.
 *
 * Counters for hit, miss and reference events should be incremented in
 * this function.
 *
 * @param mm A pointer to the mm.
 * @param access The memory access containing (among other things) the virtual
 * address.
 *
 * @return the frame number
 *
 * @pre mm is not NULL.
 */
uint64_t
CSC369_MMGetFrameNumber(CSC369_MM* mm, CSC369_MemAccess access);

/**
 * Handle an eviction.
 *
 * This function is called by CSC369_CoremapAllocateFrame. It should write the
 * victim page to swap, if needed. It should also update the page table.
 *
 * Counters for evictions should be updated appropriately in this function.
 *
 * @param swap A pointer to the mm.
 * @param victim The page being evicted.
 */
void
CSC369_MMHandleEviction(CSC369_Swap* swap, csc369_pte_t* victim);

/**
 * Get the page table being managed.
 *
 * @param mm A pointer to the mm.
 *
 * @return A pointer to the page table.
 *
 * @pre mm is not NULL.
 */
CSC369_PageTable*
CSC369_MMGetPageTable(CSC369_MM* mm);

#endif // CSC369H1_MEMORY_MM_H
