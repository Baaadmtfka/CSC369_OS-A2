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
#ifndef CSC369H1_MEMORY_PTE_H
#define CSC369H1_MEMORY_PTE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct csc369_pte_t csc369_pte_t; // forward declaration

csc369_pte_t*
CSC369_PTECreate(void);

void
CSC369_PTEDestroy(csc369_pte_t* pte);

void
CSC369_PTESetValid(csc369_pte_t* pte, bool value);

void
CSC369_PTESetDirty(csc369_pte_t* pte, bool value);

bool
CSC369_PTEIsOnSwap(csc369_pte_t* pte);

void
CSC369_PTESetOnSwap(csc369_pte_t* pte, bool value);

void
CSC369_PTESetFrameNumber(csc369_pte_t* pte, uint64_t frame_number);

off_t
CSC369_PTEGetSwapOffset(csc369_pte_t* pte);

void
CSC369_PTESetSwapOffset(csc369_pte_t* pte, uint64_t swap_offset);

#endif // CSC369H1_MEMORY_PTE_H
