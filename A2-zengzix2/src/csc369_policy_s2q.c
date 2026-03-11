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
#include "csc369_policy.h"

#include "csc369_coremap.h"
#include "csc369_def.h"
#include "csc369_logger.h"
#include "csc369_page_table.h"

#include <assert.h>
#include <stdbool.h>

// Global Vars:
static list_head A1;
static list_head Am;

void
CSC369_PolicyS2QInit(void)
{
  list_init(&A1);
  list_init(&Am);
}

void
CSC369_PolicyS2QCleanup(void)
{
  list_destroy(&A1);
  list_destroy(&Am);
}

/*
 * Algorithm from paper (frame is our frame):
 * if frame is on the Am queue
 *    put frame on the front of the Am queue (i.e. first pos in LRU queue)
 * else if frame is on the A1 queue
 *     remove frame from the A1 queue
 *     put frame on the front of the Am queue
 * else (first access we know about concerning frame)
 *     (find a free page slot for frame already done in s2q_evict)
 *     put frame on the front of the A1 queue (i.e. last pos in FIFO queue)
 */
void
CSC369_PolicyS2QOnRef(CSC369_Frame* frame)
{
  assert(frame != NULL && !frame->free);
  // pointer to current entry
  list_entry* curr_entry = &frame->framelist_entry;
  
  // first access
  if (!list_entry_is_linked(curr_entry)) {
    CSC369_LOG_TRACE("First Access FrameNumber: %d",(int)CSC369_PTEGetFrameNumber(frame->pte));
    list_add_tail(&A1, curr_entry);
  } else {
    // frame on A1 queue (if reference bit is 0)
    if (!CSC369_PTEIsReferenced(frame->pte)) {
      CSC369_LOG_TRACE("Ref A1 FrameNumber: %d",(int)CSC369_PTEGetFrameNumber(frame->pte));
      // set reference bit of the frame's PTE to true
      CSC369_PTESetReferenced(frame->pte, true);
    }
    // frame on Am queue
    else {
      // sudo branch
      CSC369_LOG_TRACE("Ref Am FrameNumber: %d",(int)CSC369_PTEGetFrameNumber(frame->pte));
    }
    // Either way, remove from current queue, put into Am
    list_del(curr_entry);
    list_add_head(&Am, curr_entry);
  }
}

/*
 * Algorithm from paper:
 *
 * if A1’s size is above a (tunable) threshold
 *     delete the oldest item in A1
 *     return freed frame
 * else
 *    delete the least-recently-used item in Am
 *    return freed frame
 */
uint64_t
CSC369_PolicyS2QEvict(CSC369_Frame* all_frames)
{
  assert(all_frames != 0);
  
  // size of A1
  list_entry* pos;
  size_t i = 0;
  list_for_each(pos, &A1) {
    i++;
  }
  
  list_entry* evict_entry;
  CSC369_Frame* evict_frame;
  // A1 size above threshold
  if (i > CSC369_GLOBAL_MEMSIZE / 10) {
    evict_entry = list_first_entry(&A1);
  } else {
    evict_entry = list_last_entry(&Am);
  }
  evict_frame = container_of(evict_entry, CSC369_Frame, framelist_entry);
  CSC369_LOG_TRACE("Evict FrameNumber: %d",(int)CSC369_PTEGetFrameNumber(evict_frame->pte));
  list_del(evict_entry);
  evict_entry->next = NULL;
  evict_entry->prev = NULL;
  CSC369_PTESetReferenced(evict_frame->pte, false);
  
  return CSC369_PTEGetFrameNumber(evict_frame->pte);
}
