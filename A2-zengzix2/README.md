# Assignment - Virtual Memory

This README describes how to compile this project and run the compiled executables.
For the instructions on what to do for this exercise, please see [the handout](HANDOUT.md).

## Dependencies

This project uses CMake to configure and build its binaries.
The `teach.cs` machines have CMake version 3.22 already installed.
But if you are working locally, you will need to install CMake as well.

The project uses the [Check unit testing framework](https://github.com/libcheck/check).
The `teach.cs` machines have version 0.15.2 installed (see `https://github.com/libcheck/check`).
If you are working locally, you will need to install this as well.
You should build from source (it is a small library, should not take long), see the library's [CMake instructions](https://github.com/libcheck/check#installing).

## Compiling

CMake can configure the project for different build systems and IDEs (type `cmake --help` for a list of generators available for your platform).
You can also work via the command line.
We recommend you create a build directory before invoking CMake to configure the project (`cmake -B`).
For example, we can perform the configuration step from the project root directory:

```zsh
wolf:~/369/0test/A2$ cmake -H. -Bcmake-build-debug -DCMAKE_BUILD_TYPE=Debug
```

After the configuration step, you can ask CMake to build the project:

```zsh
wolf:~/369/0test/A2$ cmake --build cmake-build-debug/ --target all
```

## Running

If compilation is successful, the `memsim` and `check_page_table` binary should be created.

### Running `check_page_table`

To run the page table unit tests after compilation, you can:

```zsh
wolf:~/369/0test/A2$ cd cmake-build-debug
wolf:~/369/0test/A2/cmake-build-debug$ cd bin/
wolf:~/369/0test/A2/cmake-build-debug/bin$ ./check_page_table
```

Which produces output like:

```
Running suite(s): Page Table Test Suite
0%: Checks: 2, Failures: 2, Errors: 0
/home/mbadr/anita/369/assignment-memory/starter-files/tests/check_page_table.c:38:S:Page Table Search Tests:test_search_none_exist:0: Assertion 'page_table != NULL' failed: page_table == 0
/home/mbadr/anita/369/assignment-memory/starter-files/tests/check_page_table.c:38:S:Page Table Search Tests:test_allocate_pte:0: Assertion 'page_table != NULL' failed: page_table == 0
```


### Running `memsim`

To run the simulator, you will either need to have created a trace file, downloaded them from teach.cs, or run the simulator while SSH'd onto teach.cs.
For example, after compilation, you can:

```zsh
wolf:~/369/0test/A2$ cd cmake-build-debug
wolf:~/369/0test/A2/cmake-build-debug$ cd bin/
wolf:~/369/0test/A2/cmake-build-debug/bin$ ./memsim -f /u/csc369h/winter/pub/a2/traces/simvaddr-simpleloop.ref -m 2600 -s 1 -a RR
```

Which produces output like:

```
[INFO]  [memsim.c:107] Trace file: simvaddr-simpleloop.ref.
[INFO]  [memsim.c:111] Number of frames (physical memory): 2600.
[INFO]  [memsim.c:118] Number of frames (swap): 1.
[INFO]  [memsim.c:69] Replacement: RR.
```

Note that the simulation has immediately exited because it was unable to create a memory manager (with the starter files). 
