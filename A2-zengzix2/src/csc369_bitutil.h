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
#ifndef CSC369H1_MEMORY_BITUTIL_H
#define CSC369H1_MEMORY_BITUTIL_H
/**
 * Functions for working with 64-bit bitfields.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Check whether a bit in the bitfield is set or not.
 *
 * @param bitfield The bitfield.
 * @param bit The bit in the bitfield to check.
 *
 * @return true if the bit is 1, false otherwise.
 */
bool
CSC369_BitfieldGetBit(uint64_t bitfield, uint8_t bit);

/**
 * Set the bit in the bitfield to 1 if value is true, 0 otherwise.
 *
 * @param bitfield The bitfield.
 * @param bit The bit in the bitfield to set.
 * @param value The value to set the bit to.
 *
 * @return The new bitfield value.
 */
uint64_t
CSC369_BitfieldSetBit(uint64_t bitfield, uint8_t bit, bool value);

/**
 * Get the number represented by count bits, beginning at the least-significant
 * bit.
 *
 * @param bitfield The bitfield.
 * @param count The number of bits.
 * @param lsb The least-significant bit (inclusive).
 *
 * @return The value that the bits from lsb to lsb + count represent.
 *
 */
uint64_t
CSC369_BitfieldGetBits(uint64_t bitfield, uint8_t count, uint8_t lsb);

/**
 * Set count bits in bitfield, starting at the least-significant bit, to the
 * value specified.
 *
 * @param bitfield The bitfield.
 * @param count The number of bits.
 * @param lsb The least-significant bit (inclusive).
 * @param value The new value.
 *
 * @return The bitfield with the specified bits changed to value.
 */
uint64_t
CSC369_BitfieldSetBits(uint64_t bitfield,
                       uint8_t count,
                       uint8_t lsb,
                       uint64_t value);

/**
 * A bitmap.
 */
typedef struct csc369_bitmap_t CSC369_Bitmap;

/**
 * Create a bitmap.
 *
 * @param num_frames The size of memory.
 *
 * @return A (newly-allocated) bitmap, or NULL on error.
 */
CSC369_Bitmap*
CSC369_BitmapCreate(size_t bit_count);

/**
 * Destroy the bitmap and free up any resources it holds.
 *
 * @param bitmap A pointer to the bitmap.
 *
 * @pre bitmap is not NULL.
 */
void
CSC369_BitmapDestroy(CSC369_Bitmap* bitmap);

/**
 * Allocate a bit in the bitmap.
 */
int
CSC369_BitmapAlloc(CSC369_Bitmap* bitmap, size_t* index);

#endif // CSC369H1_MEMORY_BITUTIL_H
