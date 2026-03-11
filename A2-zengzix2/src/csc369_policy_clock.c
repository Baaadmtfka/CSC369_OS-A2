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

// Global Vars:
// Pointer to the head of list
static list_head* list = NULL;
// clock hand pointer
static list_entry* hand;

void
CSC369_PolicyClockInit(void)
{
  // None
}

void
CSC369_PolicyClockCleanup(void)
{
  list_destroy(list);
}

void
CSC369_PolicyClockOnRef(CSC369_Frame* frame)
{
  assert(frame != NULL);
  CSC369_LOG_TRACE("Reference Frame Number: %d", (int)CSC369_PTEGetFrameNumber(frame->pte));
  // set reference bit of the frame's PTE to true
  CSC369_PTESetReferenced(frame->pte, true);
  
  // pointer to current entry
  list_entry* curr_entry = &frame->framelist_entry;
  
  // empty list, init
  if (list == NULL) {
    CSC369_LOG_TRACE("Head Frame Number: %d",(int)CSC369_PTEGetFrameNumber(frame->pte));
    list_entry_init(curr_entry);
    list = container_of(&frame->framelist_entry, list_head, head);
    list_init(list); //wtf
    // init hand
    hand = curr_entry;
  }
  
  else {
    if (!list_entry_is_linked(curr_entry)) {
      CSC369_LOG_TRACE("Link Frame Number: %d",(int)CSC369_PTEGetFrameNumber(frame->pte));
      list_entry_init(curr_entry);
      list_add_tail(list, curr_entry);
      hand = curr_entry;
    } else {
      // skip if entry already in linked list
      CSC369_LOG_TRACE("Linked Frame Number: %d",(int)CSC369_PTEGetFrameNumber(frame->pte));
    }
  }
}

uint64_t
CSC369_PolicyClockEvict(CSC369_Frame* all_frames)
{
  UNUSED(all_frames);
  assert(all_frames != NULL);
  // update hand, which was at last allocated frame
  hand = hand->next;
  
  // clock
  while (true) {
    //CSC369_LOG_TRACE("loop start");
    CSC369_Frame* curr_frame = container_of(hand, CSC369_Frame, framelist_entry);
    
    // If the current frame's PTE is not referenced, evict it
    if (!CSC369_PTEIsReferenced(curr_frame->pte)) {
      CSC369_LOG_TRACE("Evict FrameNumber: %d",(int)CSC369_PTEGetFrameNumber(curr_frame->pte));
      
      // update list
      list = container_of(hand->next, list_head, head);
      // evict also from linked list
      list_entry* temp_entry = hand;
      list_del(temp_entry);
      temp_entry->next = NULL;
      temp_entry->prev = NULL;
      
      // evicting head & only element
      if (hand == &list->head) {
        list = NULL;
      }
      
      return CSC369_PTEGetFrameNumber(curr_frame->pte);
    }

    // Clear reference bit of current frame
    CSC369_LOG_TRACE("Dereference FrameNumber: %d",(int)CSC369_PTEGetFrameNumber(curr_frame->pte));
    CSC369_PTESetReferenced(curr_frame->pte, false);
    
    // Move to the next frame
    hand = hand->next;
    //CSC369_LOG_TRACE("loop end");
  }
  
  // should never reach
  return 0;
}
