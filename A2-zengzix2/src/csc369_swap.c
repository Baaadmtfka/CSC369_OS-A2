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
#include "csc369_swap.h"

#include "csc369_bitutil.h"
#include "csc369_def.h"
#include "csc369_logger.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct csc369_swap_t
{
  int fd;
  CSC369_Bitmap* swapmap;
  char fname[20];

  uint8_t* physmem;
} CSC369_Swap;

CSC369_Swap*
CSC369_SwapCreate(size_t size, uint8_t* physmem)
{
  CSC369_Swap* swap = calloc(1, sizeof(CSC369_Swap));
  if (swap == NULL) {
    CSC369_LOG_ERRNO("calloc");
    return NULL;
  }

  swap->swapmap = CSC369_BitmapCreate(size);
  if (swap->swapmap == NULL) {
    CSC369_SwapDestroy(swap, true);
    return NULL;
  }

  strncpy(swap->fname, "swapfile.XXXXXX", sizeof(swap->fname));
  if ((swap->fd = mkstemp(swap->fname)) == -1) {
    CSC369_LOG_ERRNO("mkstemp");
    CSC369_SwapDestroy(swap, true);
    return NULL;
  }

  swap->physmem = physmem;

  return swap;
}

void
CSC369_SwapDestroy(CSC369_Swap* swap, bool free_memory)
{
  assert(swap != NULL);

  // Close and remove swapfile
  close(swap->fd);
  unlink(swap->fname);

  // We might call swap_destroy from signal handler, to clean up
  // temporary swapfile when process exits. If so, it is not
  // safe to call free(), and any memory leaks don't matter since
  // process will be exiting anyway.
  if (free_memory) {
    if (swap->swapmap != NULL) {
      CSC369_BitmapDestroy(swap->swapmap);
    }

    free(swap);
  }
}

int
CSC369_SwapPageIn(CSC369_Swap* swap, unsigned int frame, off_t offset)
{
  assert(offset != CSC369_INVALID_SWAP);

  // Seek to position in swap file where this page was stored
  off_t pos = lseek(swap->fd, offset, SEEK_SET);
  if (pos != offset) {
    assert(pos == (off_t)-1);
    perror("swap_pagein: failed to set read position");
    return -errno;
  }

  // Get pointer to page data in (simulated) physical memory
  void* frame_ptr = &swap->physmem[frame * CSC369_SIMULATED_FRAME_SIZE];

  // Read page data from swapfile into memory
  ssize_t bytes_read = read(swap->fd, frame_ptr, CSC369_SIMULATED_FRAME_SIZE);
  if (bytes_read != CSC369_SIMULATED_FRAME_SIZE) {
    CSC369_LOG_WARNING("swap_pagein: did not read whole page.");
    return (int)bytes_read;
  }

  return 0;
}

off_t
CSC369_SwapPageOut(CSC369_Swap* swap, unsigned int frame, off_t offset)
{
  // Check if swap has already been allocated for this page
  if (offset == CSC369_INVALID_SWAP) {
    size_t idx;
    if (CSC369_BitmapAlloc(swap->swapmap, &idx) != 0) {
      CSC369_LOG_WARNING("swap_pageout: Could not allocate space in swapfile. "
                         "Try running again with a larger swapsize.");
      return CSC369_INVALID_SWAP;
    }
    offset = ((off_t)idx) * CSC369_SIMULATED_FRAME_SIZE;
  }
  assert(offset != CSC369_INVALID_SWAP);

  // Seek to position in swap file where this page will be stored
  off_t pos = lseek(swap->fd, offset, SEEK_SET);
  if (pos != offset) {
    assert(pos == (off_t)-1);
    CSC369_LOG_ERRNO("swap_pageout: failed to set write position");
    return CSC369_INVALID_SWAP;
  }

  // Get pointer to page data in (simulated) physical memory
  void* frame_ptr = &swap->physmem[frame * CSC369_SIMULATED_FRAME_SIZE];

  // Read page data from swapfile into memory
  ssize_t bytes_written =
    write(swap->fd, frame_ptr, CSC369_SIMULATED_FRAME_SIZE);
  if (bytes_written != CSC369_SIMULATED_FRAME_SIZE) {
    CSC369_LOG_WARNING("swap_pageout: did not write whole page\n");
    return CSC369_INVALID_SWAP;
  }
  return offset;
}
