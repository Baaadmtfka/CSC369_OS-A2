# CSC369 – Assignment 2: Virtual Memory

Individual assignment for University of Toronto's CSC369 (Operating Systems): implementing a demand-paged virtual memory manager in C — a multi-level page table, a physical-memory coremap, disk-backed swap, and the Clock and Simplified 2Q (S2Q) page-replacement algorithms.

## Layout

- **`A2-zengzix2/`** — the actual submission: source (`src/`), page-table unit tests (`tests/`), CMake build config, and the course-provided [HANDOUT.md](A2-zengzix2/HANDOUT.md) / [README.md](A2-zengzix2/README.md) (build & run instructions).
- **`A2 handout.txt`** — plain-text copy of the assignment handout.

There's no written report for this assignment — it's implementation-only, graded via the page-table unit tests and `memsim` trace-driven simulation.

## What's implemented (`A2-zengzix2/src/`)

- **Page table** (`csc369_page_table.c`): a 3-level page table over the virtual address, splitting it into three 12-bit indices (bits 36+, 24–35, 12–23) to walk/allocate nested tables of `csc369_pte_t` entries, each tracking a frame number, swap offset, and valid/dirty/referenced/on-swap flags.
- **Memory manager** (`csc369_mm.c`): `CSC369_MMGetFrameNumber` is the core address-translation path — on a PTE miss it allocates a coremap frame, either zero-fills it (page never touched) or swaps the page back in, then marks the PTE valid; on a hit it just returns the cached frame number. Store/modify accesses mark the PTE dirty. `CSC369_MMHandleEviction` writes a victim page out to swap (tracking clean vs. dirty eviction counts) before its frame is reused.
- **Coremap** (`csc369_coremap.c`): tracks which physical frames are free vs. in use. `CSC369_CoremapAllocateFrame` hands out a free frame if one exists, otherwise calls the active eviction policy (`CSC369_Evict`) to pick a victim frame and evicts it first.
- **Swap** (`csc369_swap.c`): backs evicted pages with a temp file on disk (`mkstemp`), tracked via a bitmap of free/used swap slots.
- **Eviction policies** (`csc369_policy_*.c`), selected at runtime:
  - **Clock**: a circular list of in-use frames with a "hand" pointer; sweeps forward clearing reference bits until it finds an unreferenced frame to evict.
  - **S2Q**: two queues, a FIFO `A1` for frames seen once and an LRU `Am` for frames referenced again; evicts from the head of `A1` once it exceeds a size threshold (memory size / 10), otherwise evicts the LRU tail of `Am`.
  - **RR** / **Rand**: simpler round-robin and random victim selection, for comparison.
- **Support utilities**: `csc369_bitutil.c` (bitmap get/set/find-first-free) backs the swap-slot allocator; `csc369_kmalloc.c` wraps allocation with tracking for the test suite.

## Building & running

See [A2-zengzix2/README.md](A2-zengzix2/README.md) for CMake build instructions, running the `check_page_table` unit tests, and running `memsim` against a memory-access trace with a chosen replacement policy (`-a RR|Rand|Clock|S2Q`).

## Author

Zixuan Zeng
