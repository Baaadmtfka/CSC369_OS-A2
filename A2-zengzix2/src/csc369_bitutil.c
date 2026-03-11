/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC369H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2024 Mario Badr
 */
#include "csc369_bitutil.h"

#include "csc369_logger.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/**
 * Mask BIT_COUNT bits starting from BIT_START.
 */
#define CSC369_CREATE_MASK(BIT_COUNT, BIT_START)                               \
  (~(0xffffffffffffffff << (BIT_COUNT)) << (BIT_START))

bool
CSC369_BitfieldGetBit(uint64_t bitfield, uint8_t bit)
{
  // 2 to the power of bit
  uint64_t const mask = 1L << bit;
  return bitfield & mask;
}

uint64_t
CSC369_BitfieldSetBit(uint64_t bitfield, uint8_t bit, bool value)
{
  // 2 to the power of bit
  uint64_t const mask = 1L << bit;

  if (value == true) {
    bitfield |= mask;
  } else {
    bitfield &= ~mask;
  }

  return bitfield;
}

uint64_t
CSC369_BitfieldGetBits(uint64_t bitfield, uint8_t count, uint8_t lsb)
{
  uint64_t const mask = CSC369_CREATE_MASK(count, lsb);
  uint64_t const masked_bits = bitfield & mask;
  return masked_bits >> lsb;
}

uint64_t
CSC369_BitfieldSetBits(uint64_t bitfield,
                       uint8_t count,
                       uint8_t lsb,
                       uint64_t value)
{
  uint64_t const mask = CSC369_CREATE_MASK(count, lsb);

  // Place the bits representing the value into the right position
  value = (value << lsb) & mask;
  // Zero-out the part of the bit field we care about
  bitfield = bitfield & ~mask;
  // Put the requested value into the bitfield
  return bitfield | value;
}

// The bitmap code below is modified from the OS/161 bitmap functions.
static const size_t bits_per_word = sizeof(size_t) * CHAR_BIT;
static const size_t word_all_bits = (size_t)-1;

static size_t
nwords_for_nbits(size_t nbits)
{
  return (nbits + bits_per_word - 1) / bits_per_word;
}

typedef struct csc369_bitmap_t
{
  size_t nbits;
  size_t* words;
} CSC369_Bitmap;

CSC369_Bitmap*
CSC369_BitmapCreate(size_t bit_count)
{
  CSC369_Bitmap* b = calloc(1, sizeof(CSC369_Bitmap));
  if (b == NULL) {
    CSC369_LOG_ERRNO("calloc");
    return NULL;
  }

  size_t word_count = nwords_for_nbits(bit_count);
  b->words = calloc(word_count, sizeof(size_t));
  if (!b->words) {
    CSC369_LOG_ERRNO("calloc");
    CSC369_BitmapDestroy(b);
    return NULL;
  }

  memset(b->words, 0, word_count * sizeof(size_t));
  b->nbits = bit_count;

  // Mark any leftover bits at the end in use
  if (word_count > bit_count / bits_per_word) {
    size_t idx = word_count - 1;
    size_t overbits = bit_count - idx * bits_per_word;

    assert(bit_count / bits_per_word == word_count - 1);
    assert(overbits > 0 && overbits < bits_per_word);

    for (size_t j = overbits; j < bits_per_word; ++j) {
      b->words[idx] |= ((size_t)1 << j);
    }
  }

  return b;
}

void
CSC369_BitmapDestroy(CSC369_Bitmap* bitmap)
{
  if (bitmap->words != NULL) {
    free(bitmap->words);
  }

  free(bitmap);
}

int
CSC369_BitmapAlloc(CSC369_Bitmap* bitmap, size_t* index)
{
  size_t max_idx = nwords_for_nbits(bitmap->nbits);

  for (size_t idx = 0; idx < max_idx; ++idx) {
    if (bitmap->words[idx] != word_all_bits) {
      for (size_t offset = 0; offset < bits_per_word; ++offset) {
        size_t mask = (size_t)1 << offset;

        if ((bitmap->words[idx] & mask) == 0) {
          bitmap->words[idx] |= mask;
          *index = (idx * bits_per_word) + offset;
          assert(*index < bitmap->nbits);
          return 0;
        }
      }
      assert(false);
    }
  }
  return -1;
}
