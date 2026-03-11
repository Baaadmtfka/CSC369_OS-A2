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
#ifndef CSC369H1_MEMORY_POLICY_H
#define CSC369H1_MEMORY_POLICY_H

#include <stddef.h>
#include <stdint.h>

typedef struct csc369_frame_t CSC369_Frame; // forward declaration

/**
 * An algorithm that implements a replacement policy.
 */
typedef struct csc369_policy_t
{
  /**
   * The name of the replacement policy.
   */
  char const* name;

  /**
   * Initialize any data structures or variables needed for this policy.
   */
  void (*CSC369_PolicyInit)(void);

  /**
   * Cleanup any data structures created in the init function.
   */
  void (*CSC369_PolicyCleanup)(void);

  /**
   * This function is called on each access to a page.
   *
   * @param frame The frame that was referenced.
   */
  void (*CSC369_PolicyOnRef)(CSC369_Frame* frame);

  /**
   * Return the frame number for the page that is to be evicted.
   *
   * @param all_frames An array of all frames.
   */
  uint64_t (*CSC369_PolicyEvict)(CSC369_Frame* all_frames);
} CSC369_Policy;

// The names of each replacement policy is in the parentheses
#define CSC369_POLICY_NAMES                                                    \
  CSC369_POLICY_DECL(Rand)                                                     \
  CSC369_POLICY_DECL(RR)                                                       \
  CSC369_POLICY_DECL(Clock)                                                    \
  CSC369_POLICY_DECL(S2Q)

// This macro uses the one above to declare the functions needed for each
// replacement policy
#define CSC369_POLICY_DECL(name)                                               \
  void CSC369_Policy##name##Init(void);                                        \
  void CSC369_Policy##name##Cleanup(void);                                     \
  void CSC369_Policy##name##OnRef(CSC369_Frame* frame);                        \
  uint64_t CSC369_Policy##name##Evict(CSC369_Frame*);
CSC369_POLICY_NAMES
#undef CSC369_POLICY_DECL

#endif // CSC369H1_MEMORY_POLICY_H
