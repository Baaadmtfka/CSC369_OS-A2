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
#ifndef CSC369H1_MEMORY_KMALLOC_H
#define CSC369H1_MEMORY_KMALLOC_H

#include <stdbool.h>
#include <stddef.h>

void*
CSC369_malloc(size_t size);

void
CSC369_free(void* ptr);

void
CSC369_MallocInit(bool verbose);

long
CSC369_MallocCurrentBytes(void);

long
CSC369_MallocCurrentCount(void);

long
CSC369_MallocBytes();

long
CSC369_MallocCount();

bool
CSC369_MallocIsLeakFree(long num_mallocs_tol, long num_bytes_tol);

#endif // CSC369H1_MEMORY_KMALLOC_H
