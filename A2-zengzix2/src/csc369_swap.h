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
#ifndef CSC369H1_MEMORY_SWAP_H
#define CSC369H1_MEMORY_SWAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define CSC369_INVALID_SWAP ((off_t)-1)

/**
 * Manages the swap space.
 */
typedef struct csc369_swap_t CSC369_Swap;

/**
 * Create a swap space manager.
 *
 * @param size The size of the swap space.
 * @param physmem A pointer to the physical memory.
 *
 * @return A newly-allocated swap space manager, or NULL on error.
 *
 * @pre physmem is not NULL.
 */
CSC369_Swap*
CSC369_SwapCreate(size_t size, uint8_t* physmem);

/**
 * Destroy a swap space manager and any resources it holds.
 *
 * @param swap A pointer to the swap space manager.
 * @param free_memory whether or not memory (in addition to files) should be
 * freed.
 *
 * @pre swap is not NULL.
 */
void
CSC369_SwapDestroy(CSC369_Swap* swap, bool free_memory);

/**
 * Read data into the (simulated) physical memory frame from the offset in the
 * swap file.
 *
 * @param swap A pointer to the swap space manager.
 * @param frame The physical frame number of physical memory.
 * @param offset The byte position in the swap file.
 *
 * @return 0 on success, -errno on error, number of bytes on partial read
 *
 * @pre swap is not NULL.
 */
int
CSC369_SwapPageIn(CSC369_Swap* swap, unsigned int frame, off_t offset);

/**
 * Write the data from the (simulated) physical memory frame to the offset in
 * the swap file.
 *
 * Allocates space in the swap file for a virtual page, if needed, by specifying
 * CSC369_INVALID_SWAP as the offset.
 *
 * @param swap A pointer to the swap space manager.
 * @param frame The physical frame number of physical memory.
 * @param offset The byte position in the swap file.
 *
 * @return the offset where the data was written on success, CSC369_INVALID_SWAP
 * otherwise
 *
 * @pre swap is not NULL.
 */
off_t
CSC369_SwapPageOut(CSC369_Swap* swap, unsigned int frame, off_t offset);

#endif // CSC369H1_MEMORY_SWAP_H
