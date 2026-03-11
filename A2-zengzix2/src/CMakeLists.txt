add_library(csc369_memory
    csc369_bitutil.c
    csc369_bitutil.h
    csc369_coremap.c
    csc369_coremap.h
    csc369_def.h
    csc369_mm.c
    csc369_mm.h
    csc369_kmalloc.c
    csc369_kmalloc.h
    csc369_logger.h
    csc369_page_table.c
    csc369_page_table.h
    csc369_pte.h
    csc369_policy.h
    csc369_policy_clock.c
        csc369_policy_rand.c
    csc369_policy_rr.c
    csc369_policy_s2q.c
    csc369_swap.c
    csc369_swap.h
    khash.h
    list.h
)

add_library(CSC369::memory ALIAS csc369_memory)

target_include_directories(csc369_memory
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_options(csc369_memory
    PRIVATE
      -Wall -Wextra -Werror
)

add_executable(memsim
    csc369_simulation.c
    csc369_simulation.h
    csc369_trace.c
    csc369_trace.h
    memsim.c
)

target_link_libraries(memsim
    PRIVATE
      CSC369::memory
)

target_compile_options(memsim
    PRIVATE
      -Wall -Wextra -Werror
)

set_target_properties(memsim
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CSC369H1_MEMORY_BIN_DIR}"
)
