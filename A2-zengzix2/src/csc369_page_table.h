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
#ifndef CSC369H1_MEMORY_PAGE_TABLE_H
#define CSC369H1_MEMORY_PAGE_TABLE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * A page table entry.
 */
typedef struct csc369_pte_t csc369_pte_t;

/**
 * Check whether the pte is valid.
 *
 * @param pte The pte to check.
 *
 * @return true if the pte is valid, false otherwise.
 */
bool
CSC369_PTEIsValid(csc369_pte_t* pte);

/**
 * Check whether the pte is dirty.
 *
 * @param pte The pte to check.
 *
 * @return true if the pte is dirty, false otherwise.
 */
bool
CSC369_PTEIsDirty(csc369_pte_t* pte);

/**
 * Check whether the pte has been referenced.
 *
 * @param pte The pte to check.
 *
 * @return true if the pte was referenced, false otherwise.
 */
bool
CSC369_PTEIsReferenced(csc369_pte_t* pte);

/**
 * Set the reference bit of the PTE to true.
 *
 * @param pte The pte to mutate.
 * @param value
 */
void
CSC369_PTESetReferenced(csc369_pte_t* pte, bool value);

/**
 * Get the frame number from the pte.
 *
 * @param pte The page table entry.
 *
 * @return The frame number stored in this pte.
 */
uint64_t
CSC369_PTEGetFrameNumber(csc369_pte_t* pte);

/**
 * A page table.
 */
typedef struct csc369_page_table_t CSC369_PageTable;

/**
 * Create a page table.
 *
 * This function should only allocate the top-level directory. No parts of the
 * other level(s) should be allocated.
 *
 * @return A (newly-allocated) page table, or NULL on error.
 */
CSC369_PageTable*
CSC369_PageTableCreate(void);

/**
 * Destroy the page table and free up any resources it holds.
 *
 * @param page_table A pointer to the page table.
 *
 * @pre page_table is not NULL.
 */
void
CSC369_PageTableDestroy(CSC369_PageTable* page_table);

/**
 * Search for the PTE that corresponds to virtual_address.
 *
 * This function should be "efficient" and result in NO memory allocations.
 *
 * @param page_table A pointer to the page table.
 * @param virtual_address The virtual address.
 *
 * @return a pointer to the PTE if it has been allocated, NULL otherwise.
 *
 * @pre page_table is not NULL. virtual_address is valid.
 */
csc369_pte_t*
CSC369_PageTableSearch(CSC369_PageTable* page_table, uint64_t virtual_address);

/**
 * If necessary, allocate a page for the virtual address.
 *
 * If the part of the page table that this pte belongs to has not yet been
 * allocated, then it (and other levels, where applicable) should be allocated
 * in this function.
 *
 * This function should not change the bits of any page table entries.
 *
 * @param page_table A pointer to the page table.
 * @param virtual_address The virtual address to find the pte.
 *
 * @return A pointer to the PTE in this page table.
 *
 * @pre page_table is not NULL. virtual_address is valid.
 */
csc369_pte_t*
CSC369_PageTableAllocate(CSC369_PageTable* page_table,
                         uint64_t virtual_address);

#endif // CSC369H1_MEMORY_PAGE_TABLE_H
