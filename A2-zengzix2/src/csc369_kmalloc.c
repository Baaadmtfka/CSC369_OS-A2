/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC369H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2023, Angela Brown, Kuei Sun
 * Copyright (c) 2024 Mario Badr
 */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "csc369_logger.h"
#include "khash.h"

/* Need 2^63 bytes malloced before these will overflow as
 * signed types, and having signs makes the math safer
 * if the accounting is wrong.
 */
long num_mallocs;    /* Total number of malloc369 calls */
long num_frees;      /* Total number of free369 calls */
long bytes_malloced; /* Total number of bytes malloced */
long bytes_freed;    /* Total number of bytes freed */

#define CSC369_KMALLOC_GB (1024 * 1024 * 1024L) /* 1 GB, signed long type */
#define CSC369_KMALLOC_MAX                                                     \
  (2 * CSC369_KMALLOC_GB) /* maximum dynamically allocated memory */

static bool verbose;

/* Add some status bits to the 'size' stored in the malloc map */
#define FREED 0x8000000000000000 /* if set, ptr has been freed already */

KHASH_MAP_INIT_INT64(ptrmap, size_t)
khash_t(ptrmap) * malloc_map;

void*
CSC369_malloc(size_t size)
{
  long signed_size;

  /* Check if allocating 'size' bytes would overflow our tracking. */
  /* On teach.cs servers, this isn't needed because the underlying
   * malloc() will fail long before we overflow a signed long.
   * But we might run this on other systems, so check anyway.
   */
  if (size >= CSC369_KMALLOC_MAX) {
    CSC369_LOG_ERROR(
      "CSC369_malloc - size must be less than %ld, requested %lu\n",
      CSC369_KMALLOC_MAX,
      size);
    return NULL;
  }
  if (((size_t)bytes_malloced + size) > CSC369_KMALLOC_MAX) {
    CSC369_LOG_ERROR(
      "CSC369_malloc - total bytes allocated must be less than %ld, "
      "with current request for %lu bytes, total would be %lu\n",
      CSC369_KMALLOC_MAX,
      size,
      ((size_t)bytes_malloced + size));
    return NULL;
  }

  signed_size = (long)size;
  void* m = malloc(signed_size);
  if (m == NULL) {
    /* nothing allocated, nothing to track */
    return m;
  }

  num_mallocs++;
  bytes_malloced += signed_size;

  /* Record the ptr for later free tracking */
  int ret;
  khiter_t k = kh_put(ptrmap, malloc_map, (size_t)m, &ret);
  assert(ret >= 0);
  if (ret == 0 && verbose) { /* key was present and not deleted */
    CSC369_LOG_TRACE("CSC369_malloc - malloc returned reused ptr\n");
  }
  kh_value(malloc_map, k) = signed_size;

  return m;
}

void
CSC369_free(void* ptr)
{
  size_t size = 0;
  bool is_missing = true;
  khiter_t k;

  if (ptr == NULL) {
    /* Ok to free(NULL) but we don't want to count that as
     * matching an actual malloc.
     */
    free(ptr);
    return;
  }

  k = kh_get(ptrmap, malloc_map, (size_t)ptr);
  is_missing = (k == kh_end(malloc_map));

  /* Get the size and check if we are trying to free an address that
   * we didn't get from malloc.
   */

  if (!is_missing) {
    size = kh_value(malloc_map, k);
  } else {
    if (verbose) {
      CSC369_LOG_WARNING("CSC369_free - trying to free a ptr that is "
                         "not in our map!");
    }
    free(ptr); /* Should abort if our map is correct. */
    return;
  }

  /* Check for double-free.
   */
  if (size & FREED) {
    if (verbose) {
      CSC369_LOG_WARNING("free of already freed ptr %p detected!", ptr);
    }
    free(ptr); /* Should abort if our check is correct */
    return;
  }

  /* Count one more free of size bytes */
  assert(size != 0);
  assert(size < LONG_MAX);
  assert(num_mallocs - num_frees > 0);
  num_frees++;
  assert((bytes_malloced - bytes_freed) >= (long)size);
  bytes_freed += size;

  /* Fill freed memory with 0xee to help detect use-after-free bugs. */
  /* Why 0xee? Because (a) filling with 0xff can look like -1 which might
   * be misleading, and (b) filling with a hex-word like '0xdead'
   * requires either an assumption that malloc'd sizes are always even
   * or more complicated code to check if size is even or odd.
   * Depending on how you look at things you may see memory containing
   * 0xee in different ways. For example, when viewed as:
   *     char:     0xee (1 byte) = -18
   *     unsigned char: 0xee (1 byte) = 238
   *     Viewed as an int:   0xeeeeeeee (4 bytes) = -286331154
   *     Viewed as unsigned: 0xeeeeeeee (4 bytes) = 4008636142
   *     Viewed as long: 0xeeeeeeeeeeeeeeee (8 bytes) = -1229782938247303442
   *     Viewed as unsigned long: 0xeeeeeeeeeeeeeeee (8 bytes) =
   * 17216961135462248174 Viewed as ptr: 0xeeeeeeeeeeeeeeee (8 bytes) =
   * 0xeeeeeeeeeeeeeeee
   *
   * Looking at memory in hex, or as (void *) type in gdb will make it
   * easy to spot the 'freed memory chunk' pattern.
   */

  char* region = (char*)ptr;
  for (size_t i = 0; i < size; i++) {
    region[i] = (char)0xee;
  }
  free(ptr);
  kh_value(malloc_map, k) |= FREED;
}

void
CSC369_MallocInit(bool verb)
{
  malloc_map = kh_init(ptrmap);
  verbose = verb;
  num_mallocs = 0;
  bytes_malloced = 0;
  num_frees = 0;
  bytes_freed = 0;
}

long
CSC369_MallocCurrentBytes()
{
  assert(bytes_malloced >= bytes_freed);
  return (bytes_malloced - bytes_freed);
}

long
CSC369_MallocCurrentCount()
{
  assert(num_mallocs >= num_frees);
  return (num_mallocs - num_frees);
}

long
CSC369_MallocCount()
{
  return num_mallocs;
}

long
CSC369_MallocBytes()
{
  return bytes_malloced;
}

/* Pass in 'tolerance' for number of mallocs and bytes malloc'd that we
 * won't consider a leak.
 */
bool
CSC369_MallocIsLeakFree(long num_mallocs_tol, long num_bytes_tol)
{
  if (CSC369_MallocCurrentBytes() > num_bytes_tol ||
      CSC369_MallocCurrentCount() > num_mallocs_tol) {
    return false;
  } else {
    return true;
  }
}