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
#include "csc369_trace.h"

#include "csc369_logger.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct csc369_trace_t
{
  FILE* file;
  char buffer[256];
  size_t line_number;
} CSC369_Trace;

CSC369_Trace*
CSC369_TraceCreate(char const* path)
{
  assert(path != NULL);

  CSC369_Trace* trace = calloc(1, sizeof(CSC369_Trace));
  if (trace == NULL) {
    CSC369_LOG_ERRNO("calloc");
    return NULL;
  }

  trace->file = fopen(path, "r");
  if (!trace->file) {
    CSC369_LOG_ERRNO("fopen");
    CSC369_TraceDestroy(trace);
    return NULL;
  }

  trace->line_number = 0;

  return trace;
}

void
CSC369_TraceDestroy(CSC369_Trace* trace)
{
  assert(trace != NULL);

  if (trace->file != NULL) {
    fclose(trace->file);
  }

  free(trace);
}

int
CSC369_TraceRead(CSC369_Trace* trace, CSC369_MemAccess* access)
{
  trace->line_number++;
  while (fgets(trace->buffer, 256, trace->file) != NULL &&
         trace->buffer[0] == '=') {
  }

  if (feof(trace->file)) {
    return -1;
  }

  char type;
  int count = sscanf(trace->buffer,
                     "%c %zx %hhu",
                     &type,
                     &access->virtual_address,
                     &access->value);
  if (count != 3) {
    CSC369_LOG_ERROR(
      "Invalid line in trace (#%zu): %s", trace->line_number, trace->buffer);
    return -1;
  }

  if (type == 'I') {
    access->type = CSC369_ACCESS_TYPE_INSTRUCTION_FETCH;
  } else if (type == 'L') {
    access->type = CSC369_ACCESS_TYPE_LOAD;
  } else if (type == 'S') {
    access->type = CSC369_ACCESS_TYPE_STORE;
  } else if (type == 'M') {
    access->type = CSC369_ACCESS_TYPE_MODIFY;
  } else {
    CSC369_LOG_ERROR("Invalid access type in trace (#%zu): %s",
                     trace->line_number,
                     trace->buffer);
    return -1;
  }

  if ((access->virtual_address % CSC369_PAGE_SIZE) >
      CSC369_SIMULATED_FRAME_SIZE) {
    CSC369_LOG_ERROR("Invalid virtual address (offset must be in range of a "
                     "simulated frame) in trace (#%zu): %s",
                     trace->line_number,
                     trace->buffer);
    return -1;
  }

  CSC369_LOG_DEBUG("[line:%zu] %c 0x%012lx %hhu",
                   trace->line_number,
                   type,
                   access->virtual_address,
                   access->value);
  return 0;
}
