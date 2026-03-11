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

#include "csc369_page_table.h"

#include "csc369_def.h"
#include "csc369_kmalloc.h"
#include "csc369_logger.h"

#include <assert.h>
#include <stdlib.h>

typedef struct csc369_pte_t
{
  uint64_t frame_number;
  off_t swap_offset;
  bool valid;
  bool dirty;
  bool reference;
  bool on_swap;
  // pointer to lower level page table if pte is in a page directory
  CSC369_PageTable* page_table;
} csc369_pte_t;

csc369_pte_t* CSC369_PTECreate(void)
{
  csc369_pte_t* pte = CSC369_malloc(sizeof(csc369_pte_t));
  if (pte == NULL) {
    return NULL;
  }
  
  // init pte
  pte->frame_number = 0;
  pte->swap_offset = -1;
  pte->valid = false;
  pte->dirty = false;
  pte->reference = false;
  pte->on_swap = false;
  pte->page_table = NULL;
  
  return pte;
}

void CSC369_PTEDestroy(csc369_pte_t* pte)
{
  if (pte->page_table != NULL) {
    CSC369_PageTableDestroy(pte->page_table);
  }
  CSC369_free(pte);
}

bool
CSC369_PTEIsValid(csc369_pte_t* pte)
{
  assert(pte != NULL);
  return pte->valid;
}

void
CSC369_PTESetValid(csc369_pte_t* pte, bool value)
{
  assert(pte != NULL);
  pte->valid = value;
}

bool
CSC369_PTEIsDirty(csc369_pte_t* pte)
{
  assert(pte != NULL);
  return pte->dirty;
}

void
CSC369_PTESetDirty(csc369_pte_t* pte, bool value)
{
  assert(pte != NULL);
  pte->dirty = value;
}

bool
CSC369_PTEIsReferenced(csc369_pte_t* pte)
{
  assert(pte != NULL);
  return pte->reference;
}

void
CSC369_PTESetReferenced(csc369_pte_t* pte, bool value)
{
  assert(pte != NULL);
  UNUSED(value);
  pte->reference = value;
}

bool
CSC369_PTEIsOnSwap(csc369_pte_t* pte)
{
  assert(pte != NULL);
  return pte->on_swap;
}

void
CSC369_PTESetOnSwap(csc369_pte_t* pte, bool value)
{
  assert(pte != NULL);
  UNUSED(value);
  pte->on_swap = value;
}

uint64_t
CSC369_PTEGetFrameNumber(csc369_pte_t* pte)
{
  assert(pte != NULL);
  return pte->frame_number;
}

void
CSC369_PTESetFrameNumber(csc369_pte_t* pte, uint64_t frame_number)
{
  assert(pte != NULL);
  pte->frame_number = frame_number;
}

off_t
CSC369_PTEGetSwapOffset(csc369_pte_t* pte)
{
  assert(pte != NULL);
  return pte->swap_offset;
}

void
CSC369_PTESetSwapOffset(csc369_pte_t* pte, off_t swap_offset)
{
  assert(pte != NULL);
  pte->swap_offset = swap_offset;
}


typedef struct csc369_page_table_t
{
  // pointer to pointers to page table entries
  csc369_pte_t** ptes;
} CSC369_PageTable;

CSC369_PageTable*
CSC369_PageTableCreate(void)
{
  // allocate page table
  CSC369_PageTable* page_table = CSC369_malloc(sizeof(CSC369_PageTable));
  if (page_table == NULL) {
    return NULL;
  }
  
  // allocate ptes
  csc369_pte_t** ptes = CSC369_malloc(sizeof(csc369_pte_t*) * \
          ((uint64_t)1 << (CSC369_VADDR_PAGE_NUMBER_BITS / 3)));
  if (ptes == NULL) {
    return NULL;
  }
  for (size_t i = 0; i < ((uint64_t)1 << (CSC369_VADDR_PAGE_NUMBER_BITS / 3)); i++) {
    //CSC369_LOG_TRACE("PTEs: %zu", i);
    ptes[i] = NULL;
  }
  page_table->ptes = ptes;
  
  return page_table;
}

void
CSC369_PageTableDestroy(CSC369_PageTable* page_table)
{
  assert(page_table != NULL);
  
  for (size_t i = 0; i < ((uint64_t)1 << 12); i++)
  {
    if (page_table->ptes[i] != NULL) {
      CSC369_PTEDestroy(page_table->ptes[i]);
    }
  }
  CSC369_free(page_table->ptes);
  CSC369_free(page_table);
}

csc369_pte_t*
CSC369_PageTableSearch(CSC369_PageTable* page_table, uint64_t virtual_address)
{
  assert(page_table != NULL);
  UNUSED(virtual_address);
  
  // level 1
  size_t page_number_1 = virtual_address / ((uint64_t)1 << 36);
  csc369_pte_t* pte_1 = page_table->ptes[page_number_1];
  if (pte_1 == NULL) {return NULL;}  // NULL PTE
  
  // level 2
  size_t page_number_2 = (virtual_address / ((uint64_t)1 << 24)) % ((uint64_t)1 << 12);
  CSC369_PageTable* page_table_2 = pte_1->page_table;
  if (page_table_2 == NULL) {  // NULL PT
    return NULL;
  }
  csc369_pte_t* pte_2 = page_table_2->ptes[page_number_2];
  if (pte_2 == NULL) {  // NULL PTE
    return NULL;
  }
  
  // level 3
  size_t page_number_3 = (virtual_address / ((uint64_t)1 << 12)) % ((uint64_t)1 << 24);
  CSC369_PageTable* page_table_3 = pte_2->page_table;
  if (page_table_3 == NULL) {  // NULL PT
    return NULL;
  }
  csc369_pte_t* pte_3 = page_table_3->ptes[page_number_3];
  if (pte_3 == NULL) {  // NULL PTE
    return NULL;
  }
  
  csc369_pte_t* pte = page_table_3->ptes[page_number_3];
  return pte;
}

csc369_pte_t*
CSC369_PageTableAllocate(CSC369_PageTable* page_table, uint64_t virtual_address)
{
  assert(page_table != NULL);
  UNUSED(virtual_address);
  //CSC369_LOG_TRACE("v_addr: %llx", (long long unsigned int)virtual_address);
  
  // level 1
  size_t page_number_1 = virtual_address / ((uint64_t)1 << 36);
  //CSC369_LOG_TRACE("page_number: %llx", (long long unsigned int)page_number_1);
  if (page_table->ptes[page_number_1] == NULL) {  // NULL PTE
    //CSC369_LOG_TRACE("PTE_1");
    csc369_pte_t* n_pte_1 = CSC369_PTECreate();
    page_table->ptes[page_number_1] = n_pte_1;
  }
  csc369_pte_t* pte_1 = page_table->ptes[page_number_1];
  
  // level 2
  size_t page_number_2 = (virtual_address / ((uint64_t)1 << 24)) % ((uint64_t)1 << 12);
  //CSC369_LOG_TRACE("page_number: %llx", (long long unsigned int)page_number_2);
  if (pte_1->page_table == NULL) {
    //CSC369_LOG_TRACE("PT_2");
    CSC369_PageTable* n_pt_2 = CSC369_PageTableCreate();
    pte_1->page_table = n_pt_2;
  }
  CSC369_PageTable* pt_2 = pte_1->page_table;
  if (pt_2->ptes[page_number_2] == NULL) {
    //CSC369_LOG_TRACE("PTE_2");
    csc369_pte_t* n_pte_2 = CSC369_PTECreate();
    pt_2->ptes[page_number_2] = n_pte_2;
  }
  csc369_pte_t* pte_2 = pt_2->ptes[page_number_2];
  
  // level 3
  size_t page_number_3 = (virtual_address / ((uint64_t)1 << 12)) % ((uint64_t)1 << 12);
  //CSC369_LOG_TRACE("page_number: %llx", (long long unsigned int)page_number_3);
  if (pte_2->page_table == NULL) {
    //CSC369_LOG_TRACE("PT_3");
    CSC369_PageTable* n_pt_3 = CSC369_PageTableCreate();
    pte_2->page_table = n_pt_3;
  }
  CSC369_PageTable* pt_3 = pte_2->page_table;
  if (pt_3->ptes[page_number_3] == NULL) {
    //CSC369_LOG_TRACE("PTE_3");
    csc369_pte_t* n_pte_3 = CSC369_PTECreate();
    pt_3->ptes[page_number_3] = n_pte_3;
  }
  
  csc369_pte_t* pte = pt_3->ptes[page_number_3];
  return pte;
}
