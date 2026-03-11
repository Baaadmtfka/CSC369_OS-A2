/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC369H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors: Mario Badr
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2024 Mario Badr
 */

#ifndef CSC369H1_MEMORY_LOGGER_H
#define CSC369H1_MEMORY_LOGGER_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

// See: https://stackoverflow.com/a/3599170
#define UNUSED(x) (void)(x)

/**
 * Set to 1 to enable trace output, 0 to disable.
 */
#define CSC369_DEBUG_ENABLE 0

/**
 * Set to 1 to enable trace output, 0 to disable.
 */
#define CSC369_TRACE_ENABLE 0

/**
 * Get the basename of __FILE__.
 *
 * See: https://stackoverflow.com/a/8488201
 */
#define CSC369_LOG_BASENAME                                                    \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/**
 * The format string to use at the beginning of every log message.
 */
#define CSC369_LOG_PREFIX "[%s]\t[%s:%d] "

/**
 * The arguments to the CSC369_LOG_PREFIX format string.
 */
#define CSC369_LOG_PREFIX_ARGS(CATEGORY) CATEGORY, CSC369_LOG_BASENAME, __LINE__

/**
 * The print function to use when logging messages.
 */
#define CSC369_LOG_PRINTF(format, ...) fprintf(stderr, format, __VA_ARGS__)

/**
 * Log a trace message.
 */
#define CSC369_LOG_DEBUG(msg, args...)                                         \
  do                                                                           \
    if (CSC369_DEBUG_ENABLE)                                                   \
      CSC369_LOG_PRINTF(                                                       \
        CSC369_LOG_PREFIX msg "\n", CSC369_LOG_PREFIX_ARGS("DEBUG"), ##args);  \
  while (0)

/**
 * Log a trace message.
 */
#define CSC369_LOG_TRACE(msg, args...)                                         \
  do                                                                           \
    if (CSC369_TRACE_ENABLE)                                                   \
      CSC369_LOG_PRINTF(                                                       \
        CSC369_LOG_PREFIX msg "\n", CSC369_LOG_PREFIX_ARGS("TRACE"), ##args);  \
  while (0)

/**
 * Log an info message.
 */
#define CSC369_LOG_INFO(msg, args...)                                          \
  CSC369_LOG_PRINTF(                                                           \
    CSC369_LOG_PREFIX msg "\n", CSC369_LOG_PREFIX_ARGS("INFO"), ##args)

/**
 * Log a warning message.
 */
#define CSC369_LOG_WARNING(msg, args...)                                       \
  CSC369_LOG_PRINTF(                                                           \
    CSC369_LOG_PREFIX msg "\n", CSC369_LOG_PREFIX_ARGS("WARN"), ##args)

/**
 * Log an error message.
 */
#define CSC369_LOG_ERROR(msg, args...)                                         \
  CSC369_LOG_PRINTF(                                                           \
    CSC369_LOG_PREFIX msg "\n", CSC369_LOG_PREFIX_ARGS("ERROR"), ##args)

/**
 * Log an errno and its textual description.
 *
 * See: https://www.mankier.com/3/errno
 */
#define CSC369_LOG_ERRNO(func)                                                 \
  CSC369_LOG_PRINTF(CSC369_LOG_PREFIX func " failed (%u): %s\n",               \
                    CSC369_LOG_PREFIX_ARGS("ERROR"),                           \
                    errno,                                                     \
                    strerror(errno))

#endif // CSC369H1_MEMORY_LOGGER_H
