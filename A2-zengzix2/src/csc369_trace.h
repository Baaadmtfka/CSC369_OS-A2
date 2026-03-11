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
#ifndef CSC369H1_MEMORY_TRACE_H
#define CSC369H1_MEMORY_TRACE_H

#include "csc369_def.h"

/**
 * An object that can parse memory accesses from a trace file.
 */
typedef struct csc369_trace_t CSC369_Trace;

/**
 * Create an object that can parse the trace file at path.
 *
 * @param path The path to the trace file.
 *
 * @return A (newly-allocated) trace.
 *
 * @pre path is not NULL.
 */
CSC369_Trace*
CSC369_TraceCreate(char const* path);

/**
 * Destroy the trace and free up any resources it holds.
 *
 * @param trace A pointer to the trace.
 *
 * @pre trace is not NULL.
 */
void
CSC369_TraceDestroy(CSC369_Trace* trace);

/**
 * Read an item from the trace file.
 *
 * @param trace The trace to read from.
 * @param[out] access The pointer to populate with what was parsed.
 *
 * @return 0 iff an item has been read, -1 otherwise.
 */
int
CSC369_TraceRead(CSC369_Trace* trace, CSC369_MemAccess* access);

#endif // CSC369H1_MEMORY_TRACE_H
