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

#include "csc369_def.h"
#include "csc369_logger.h"

void
CSC369_PolicyRRInit(void)
{
}

void
CSC369_PolicyRRCleanup(void)
{
}

void
CSC369_PolicyRROnRef(CSC369_Frame* frame)
{
  UNUSED(frame); // nothing to do
}

uint64_t
CSC369_PolicyRREvict(CSC369_Frame* all_frames)
{
  UNUSED(all_frames);

  // Since our simulated traces have only one process that never frees memory
  // regions, this is equivalent to FIFO.
  static uint64_t i = 0;

  uint64_t victim = i;
  i = (i + 1) % CSC369_GLOBAL_MEMSIZE;

  return victim;
}
