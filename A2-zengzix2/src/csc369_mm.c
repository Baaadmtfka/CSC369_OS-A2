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
#include "csc369_mm.h"

#include "csc369_coremap.h"
#include "csc369_kmalloc.h"
#include "csc369_logger.h"
#include "csc369_page_table.h"
#include "csc369_swap.h"

#include <assert.h>
#include <stdlib.h>

typedef struct csc369_mm_t
{
  CSC369_Coremap* coremap;
  CSC369_Swap* swap;
  CSC369_PageTable* page_table;
  uint8_t* physmem;
} CSC369_MM;

CSC369_MM*
CSC369_MMCreate(CSC369_Coremap* coremap, CSC369_Swap* swap, uint8_t* physmem)
{
  assert(coremap != NULL);
  assert(swap != NULL);
  assert(physmem != NULL);

  CSC369_MM* mm = CSC369_malloc(sizeof(CSC369_MM));
  if (mm == NULL) {
    return NULL;
  }
  
  mm->coremap = coremap;
  mm->swap = swap;
  mm->physmem = physmem;
  
  CSC369_PageTable* page_table = CSC369_PageTableCreate();
  if (page_table == NULL) {
    return NULL;
  }
  mm->page_table = page_table;

  return mm;
}

void
CSC369_MMDestroy(CSC369_MM* mm)
{
  assert(mm != NULL);
  CSC369_PageTableDestroy(mm->page_table);
  CSC369_free(mm);
}

uint64_t
CSC369_MMGetFrameNumber(CSC369_MM* mm, CSC369_MemAccess access)
{
  assert(mm != NULL);
  UNUSED(access);
  
  //CSC369_LOG_TRACE("frame addr: %#010x",(int)access.virtual_address);
  // allocate on NULL, otherwise equivalent to search
  csc369_pte_t* pte = CSC369_PageTableAllocate(mm->page_table, access.virtual_address);
  
  uint64_t frame_number;
  
  // PTE not valid
  if (!CSC369_PTEIsValid(pte)) {
    CSC369_GLOBAL_STATS.miss_count += 1;
    frame_number = CSC369_CoremapAllocateFrame(mm->coremap, pte);
    CSC369_PTESetFrameNumber(pte, frame_number);
    //CSC369_LOG_TRACE("Miss: Allocated frame: %d",(int)frame_number);
    
    // PTE not on swap
    if (!CSC369_PTEIsOnSwap(pte)) {
      CSC369_CoremapClearFrame(mm->physmem, frame_number);
      // update dirty
      CSC369_PTESetDirty(pte, true);
    }
    // PTE on swap
    else {
      //CSC369_LOG_TRACE("Swap in page: %d", (int)CSC369_PTEGetSwapOffset(pte));
      int swap_result =
              CSC369_SwapPageIn(mm->swap, frame_number, CSC369_PTEGetSwapOffset(pte));
      if (swap_result != 0) {
        CSC369_LOG_TRACE("FAILED_SWAP_IN:");
        if (swap_result == -errno) {
          CSC369_LOG_TRACE("error");
        } else {
          CSC369_LOG_TRACE("partial read: %d bytes", swap_result);
        }
      }
      // update on swap
      CSC369_PTESetOnSwap(pte, false);
    }
    
    // update PTE
    CSC369_PTESetValid(pte, true);
  }
  // PTE valid
  else {
    CSC369_GLOBAL_STATS.hit_count += 1;
    frame_number = CSC369_PTEGetFrameNumber(pte);
    //CSC369_LOG_TRACE("Hit: frame: %d",(int)frame_number);
  }
  
  // update dirty
  if (access.type == CSC369_ACCESS_TYPE_STORE || access.type == CSC369_ACCESS_TYPE_MODIFY) {
    CSC369_PTESetDirty(pte, true);
  }
  
  CSC369_GLOBAL_STATS.ref_count += 1;
  return frame_number;
}

CSC369_PageTable*
CSC369_MMGetPageTable(CSC369_MM* mm)
{
  assert(mm != NULL);
  return mm->page_table;
}

void
CSC369_MMHandleEviction(CSC369_Swap* swap, csc369_pte_t* victim)
{
  assert(swap != NULL);
  assert(victim != NULL);
  if (CSC369_PTEIsDirty(victim)) {
    CSC369_GLOBAL_STATS.evict_dirty_count += 1;
  } else {
    CSC369_GLOBAL_STATS.evict_clean_count += 1;
  }
  
  off_t offset =
          CSC369_SwapPageOut(swap, CSC369_PTEGetFrameNumber(victim), CSC369_PTEGetSwapOffset(victim));
  if (offset == CSC369_INVALID_SWAP) {
    CSC369_LOG_TRACE("FAILED_SWAP_IN");
  }
  // update victim
  CSC369_PTESetDirty(victim, false);
  CSC369_PTESetValid(victim, false);
  CSC369_PTESetOnSwap(victim, true);
  
  //CSC369_LOG_TRACE("Swaped out page: %d, to offset: %jd",(int)CSC369_PTEGetFrameNumber(victim), offset);
  CSC369_PTESetSwapOffset(victim, offset);
}
