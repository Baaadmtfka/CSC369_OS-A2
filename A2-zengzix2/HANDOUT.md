# Assignment - Virtual Memory

In this assignment, you investigate memory access patterns, simulate the operation of page tables and implement several page replacement algorithms.
This will give you some practice working with the mechanisms and policies we have been talking about in class.
The assignment is based on a virtual memory simulator that uses the simvaddr-*.ref memory reference traces located at /u/csc369h/fall/pub/a3/traces.
The first task is to implement virtual-to-physical address translation and demand paging using a page table design of your choice.
Then you will implement two different page replacement algorithms: simplified 2Q and Clock.

## Introduction

Log into MarkUs to create or update your repo and get the starter code.
We recommend using "Add starter files to repository".
Remember that you cannot manually create a new A2 directory in your repo or MarkUs won't see it.

After acquiring the starter files, the only files you should modify for submission are:
- `src/csc369_mm.c`
- `src/csc369_page_table.c`
- `src/csc369_pte.h`
- `src/csc369_policy_clock.c`
- `src/csc369_policy_s2q.c`

Make sure to submit your files to MarkUs before the due date;
see the Syllabus for our policy on late submissions.

### Traces

The traces from our sample programs at `/u/csc369h/winter/pub/a2/traces` will be interesting to run once you have some confidence that your program is working, but you will definitely want to create small traces by hand for testing.

The format of the traces is `reftype vaddr value` as shown in the sample below.
Note that the page offset part of the addresses are all between 0 and 15 (0xf) to fit in the reduced simulated physical page frames.
For a write reference type (S or M), the value will be written to the virtual address.
For a read reference type (L or I) the value is the expected value that should be read from the virtual address.
It should always be the same as the value in the most recent preceding write reference to the same virtual address.
We use this to check that the address translations and pagein/pageout operations are working correctly.

A sample trace snippet is shown below:

    S 309001 182
    S 1fff000000 55
    I 108005 0
    S 308008 122
    L 1fff000000 55
    L 308008 122
    I 4cc5000 0
    L 5018008 0

Note that in our traces, the Instruction reference type is likely to always have a value of 0.
This is because these addresses are not written to after the program starts executing.
You will also see Load references with a value of 0 when the trimmed trace includes a Load from an address that has not yet been written to.

### Simulation

The main driver for the memory simulator, `memsim.c`, reads memory reference traces in the format produced by a `simify-trace.py` tool from trimmed, reduced `valgrind` memory traces.
Refer to the tutorial exercise for more information on how the traces are generated.
For each line in the trace, the program asks for the simulated physical page frame that corresponds to the given virtual address by calling find\_physpage, and then reads from the simulated physical memory at the location given by the physical frame number and the page offset. If the access type is a write ('M' for modify or 'S' for store), it will also write the value from the trace to the location. _You should read `sim.c` so that you understand how it works but you should not modify it._

The simulator is executed as `./memsim -f <tracefile> -m <memory size> -s <swapfile size> -a <replacement algorithm>` where:
- tracefile is a path to the trace file
- memory size is the number of frames of (simulated physical memory)
- swapfile size is the number of pages that can be stored in the swapfile.

  *Note: the swapfile size should be as large as the number of unique virtual pages in the trace.
  You should be able to determine easily based on your analysis from tutorial.*
- replacement algorithm is the name of the replacement policy

There are three main data structures that are used in the simulation, with lots of functionality already implemented for you:

1.  Physical Memory: This is the space for our simulated physical memory.
    We define a simulated page frame size of `CSC369_SIMULATED_FRAME_SIZE`.
    And we allocate `CSC369_SIMULATED_FRAME_SIZE * "memory size"` bytes for physical memory.
2.  Coremap: The coremap contains an array of frames that represents the state of (simulated) physical memory.
    Each frame records whether the physical frame is in use and, if so, a pointer to the page table entry for the virtual page that is using it.
3.  Swap: The swapfile functions are all implemented in this file.
    The implementation uses a bitmap to track free and used space in the swap file. 
    And functions are provided to swap pages in and out of the swap space.

    *Note*: The simulator creates a temporary file in the current directory where it is executed to use as the swapfile.
    The simulator removes this file as part of the cleanup when it completes.
    And it makes an attempt to remove the temporary file if the simulator crashes or exits early due to a detected error.
    But you should double-check in case you need to _manually remove `swapfile.XXXXXX` files._
 
### Logging

If you would like to print to stdout or stderr, please make sure you use `csc369_logger.h`.
Otherwise, our automated tests may fail.
Note that our logger now includes flags to enable/disable the "DEBUG" and "TRACE" levels.
They are both set to 0 by default, so if you would like to enable one (or both) you must set them to 1.
In our tests, we replace the `csc369_logger.h` with our own version to silence your output.

The simulator writes a value into the simulated physical memory pages for Store or Modify references.
Later, it checks that simulated physical memory contains the last written value on Load or Instruction references.
If there is a mismatch, the simulator logs an error message.
These errors indicate that there is something wrong with the address translation implementation.
You should **not** ignore these log messages.

## Part 1 - The Page Table 

*Files to be modified:* `csc369_page_table.c`, `csc369_pte.h`

In Part 1, you implement virtual-to-physical address translation and demand paging using a pagetable design of your choice. 
The format of a page table entry is up to you.
But at a minimum it must record: (1) the frame number if the virtual page is in (simulated) physical memory and (2) an offset into the swap file if the page has been written out to swap.
It must also contain flags to represent whether the entry is Valid, Dirty, and Referenced.

In `csc369_pagetable.h`, you will see functions prefixed with `CSC369_PTE` that provide setters/getters to the aforementioned fields.
However, these are *only* the functions needed by the starter files.
As you implement your solution, you will likely need more.
You should declare other `CSC369_PTE` functions in `csc369_pte.h` and implement them in `csc369_page_table.c`.

After you have decided on some format for your page table entries, you can begin to design and implement the page table itself.
See the functions declared in `csc369_page_table.h` prefixed with `CSC369_PageTable` for the functions to implement.
Note that these functions do not impose a specific design; the design is up to you.
However, you must:
1. Not rely on global state. That is, it should be possible to create more than one page table using `CSC369_PageTableCreate`, and each page table should not impact others.
2. You must use `csc369_malloc` and `csc369_free` (see `csc369_kmalloc.h`) when dynamically allocating memory.

### Efficiency

In a real operating system implementation, the memory space taken up for your page tables reduces the memory space available to store the pages of processes' virtual address spaces.
Hence, keeping page tables small is desirable.
Reducing the time complexity of page table lookups is also important.
Your solution will be evaluated on correctness as well as space and time efficiency.

### Testing your page table

A very bare `check_page_table.c` is provided in the `tests` directory.
We are not assessing you on your unit tests.
However, we highly recommend using this file to test the correctness of your implementation.
An example is included to get you started.

## Part 2 - The Memory Manager

*Files to be modified:* `csc369_mm.c`

In Part 2, you implement the functions necessary for a `memsim` simulation to run.
Before you begin this part, you will want to become familiar with the coremap, swap, and simulator code already given to you.
The core of the memory management is done in `CSC369_MMGetFrameNumber`, which is called by the simulator.
The simulator will only call your memory manager (i.e., MM) from a single trace, so you only need to manage a single page table.

If you implement `CSC369_MMGetFrameNumber` correctly, it will begin allocating frames and eventually physical memory will be full.
At this point, the coremap will call on your `CSC369_MMHandleEviction` function for help.
For both the aforementioned functions, you will need to also keep track of various counters (stats), such as hits and misses.

When a page is being evicted, there should be only 2 possibilities:
(i) the page is dirty and needs to be written to the swap;
and (ii) the page is clean and already has a copy in the swap.
Note that a newly initialized page (zero-filled) should be marked dirty on the very first access.

### Efficiency

Just like in Part 1, any dynamic allocations you do should be through `csc369_kmalloc.h`.
And keep in mind that efficiency is also a goal in Part 2.

### Testing your memory manager

We recommend creating some very small traces to begin testing your memory manager.
While you can do this by copy-pasting a small portion of one of our traces, it would be better to craft your own.
Specifically, think of a sequence of accesses that you would like to test, and see if the counters (i.e., hits and misses) match with your own understanding.
Two replacement policies are given to you already to get you started without needing to complete Part 3.

In addition to the traces, consider what the arguments to `memsim` should be.
For example, if you would like to test whether your MM uses your page table correctly to perform address translations, use a swap size of 1 and a memory size equal to the unique number of pages needed by the trace.
This way, you avoid evictions and focus only on allocating pages to frames.

To test whether you are handling evictions correctly, try the opposite approach.
Use a memory size that is smaller than the number of unique pages in the trace.
But remember that now your swap size needs to be appropriately large.
If it is too small, then you may end up in a situation where the swap space is full but you need to evict a page.
There is nothing you can do in this scenario but exit, and note that our traces never free any space.

## Part 3 - replacement policies

*Files to be modified:* `csc369_clock.c`, `csc369_s2q.c`

Using the starter code, implement the Clock (with one ref-bit) and simplified 2Q (S2Q) replacement algorithms.
We highly recommend reviewing `list.h` to help you with the implementation;
each `CSC369_Frame` has a `framelist_entry` that you may find useful.
Note that, to test your page replacement algorithms, we replace your `pagetable.c` with a solution version, so your page replacement algorithm must be contained to the provided functions.
We also recommend avoiding dynamic memory allocation in your implementations whenever possible.
Consider using global variables at the file scope, instead.

Do not add any replacement algorithm-specific code to your page table.
Your replacement algorithms must use the "Referenced" flag stored in the page table entry.
There are functions in `csc369_page_table.c` to get the values of fields in a page table entry, which you should implement (see Part 1).
Use these functions in your replacement algorithm implementations if you need to check any of these flags and do not assume a particular format for page table entries.
Otherwise, your replacement algorithms are unlikely to work with our solution version of a page table.

### Efficiency

Page replacement algorithms must be fast, since page replacement operations can be critical to performance.
Consequently, you must implement these policies with efficiency in mind.  
Here are the expected complexities for the policies:
*   RR: init, evict, ref: O(1) in time and space
*   Clock: init, ref: O(1) in time and space; evict: O(M) in time, O(1) in space, where M = size of memory
*   S2Q: init, evict, ref: O(1) in time and space

### Testing 

As in Part 2, we highly recommend generating some small traces to debug with.
Once you are confident in your implementation, you can turn to the provided `simvaddr-*.ref` traces.

## Submission

Once you have tested your code and verified that it works according to specification, **and committed it locally** (check that by running git status), you can git push it back to MarkUs.
We collect and grade the last version pushed to MarkUs after the assignment deadline.
Our tester expects files to be in the directory given to you in the starter file.
You must make sure they are submitted to the correct locations as part of the submission process.
