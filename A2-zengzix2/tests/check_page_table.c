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
#include "check.h"

#include <stdlib.h>
#include <unistd.h>

#include "csc369_def.h"
#include "csc369_kmalloc.h"
#include "csc369_page_table.h"

#define CSC369_EXIT_CODE 369

CSC369_PageTable *page_table = NULL;

static void
tear_down(void)
{
  CSC369_PageTableDestroy(page_table);
}

static void
set_up(void)
{
  CSC369_MallocInit(true);
  page_table = CSC369_PageTableCreate();
  ck_assert_ptr_nonnull(page_table);
}

START_TEST(test_search_none_exist)
{
  uint64_t virtual_address = 0x0 + 42;
  csc369_pte_t *pte = CSC369_PageTableSearch(page_table, virtual_address);
  ck_assert_ptr_null(pte);

  _exit(CSC369_EXIT_CODE);
}

START_TEST(test_allocate_pte)
{
  uint64_t virtual_address = CSC369_PAGE_SIZE + 31;
  csc369_pte_t *allocated_pte = CSC369_PageTableAllocate(page_table, virtual_address);
  ck_assert_ptr_nonnull(allocated_pte);

  ck_assert_int_eq(CSC369_PTEIsValid(allocated_pte), false);
  ck_assert_int_eq(CSC369_PTEIsDirty(allocated_pte), false);
  ck_assert_int_eq(CSC369_PTEIsReferenced(allocated_pte), false);
  ck_assert_int_eq(CSC369_PTEGetFrameNumber(allocated_pte), 0);

  _exit(CSC369_EXIT_CODE);
}

int
main(void)
{
  TCase* page_table_search = tcase_create("Page Table Search Tests");
  tcase_add_checked_fixture(page_table_search, set_up, tear_down);
  tcase_add_exit_test(page_table_search, test_search_none_exist, CSC369_EXIT_CODE);
  tcase_add_exit_test(page_table_search, test_allocate_pte, CSC369_EXIT_CODE);

  Suite* suite = suite_create("Page Table Test Suite");
  suite_add_tcase(suite, page_table_search);

  SRunner* suite_runner = srunner_create(suite);
  srunner_run_all(suite_runner, CK_VERBOSE);
  srunner_free(suite_runner);

  return EXIT_SUCCESS;
}
