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

#include <stdlib.h>

void
CSC369_PolicyRandInit(void)
{
}

void
CSC369_PolicyRandCleanup(void)
{
}

void
CSC369_PolicyRandOnRef(CSC369_Frame* frame)
{
  UNUSED(frame); // nothing to do
}

uint64_t
CSC369_PolicyRandEvict(CSC369_Frame* all_frames)
{
  UNUSED(all_frames);

  // NOTE: We keep the default seed (don't call srandom) for repeatable results
  return random() % CSC369_GLOBAL_MEMSIZE;
}
