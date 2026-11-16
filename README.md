# my_ls

A reimplementation of the Unix `ls` command in C, supporting multiple flags and directory listing behaviors.

---

## Description

`my_ls` is a custom implementation of the Unix `ls` utility written in C.

It is designed for practicing system programming concepts such as:
- file system traversal
- directory handling
- sorting algorithms
- working with Unix system calls

The goal is to replicate core functionality of `ls` while keeping the implementation clean and educational.

---

## Features

- Basic file and directory listing
- Multiple output formats:
  - long format (`-l`)
  - single column (`-1`)
  - comma-separated (`-m`)
  - horizontal and column layout (`-x`, `-C`)
- Hidden files support:
  - `-a`, `-A`
- Sorting options:
  - by name
  - by size (`-S`)
  - by time (`-t`, `-u`, `-c`)
  - reverse order (`-r`)
- Recursive listing (`-R`)
- File type indicators (`-F`, `-p`)
- Metadata display:
  - inode (`-i`)
  - numeric UID/GID (`-n`)
  - no owner/group (`-g`, `-o`)
  - block sizes (`-s`)
- Symbolic link handling (`-L`)
- Multiple arguments support
- Error handling for missing files and directories

---

## Notes

- Human-readable mode (`-h`) is implemented, but its rounding behavior may differ from GNU `ls`.
  In some edge cases, size formatting may not exactly match GNU coreutils behavior.

---

## Project Structure

```
my_ls/
├── build.sh                    # build helper (build / rebuild / clean / docs)
├── run_tests.sh                # build and run the test suite (Docker on macOS)
├── Doxyfile                    # Doxygen configuration
├── Makefile
├── main.c                      # entry point: calls ls()
├── include/
│   ├── ls.h                    # public API: ls(argc, argv)
│   └── ls/
│       ├── error.h             # ls_error codes
│       ├── file_scan.h         # stat/readdir -> file_data vectors
│       ├── sorting.h           # ordering of entries
│       ├── flags/
│       │   ├── flags.h         # struct ls_flags (parsed options)
│       │   ├── flag_parser.h   # table-driven flag parser
│       │   └── flag_setters.h  # one setter per flag, 256-entry table
│       └── print/
│           ├── print.h         # ls_print_catalog(): format dispatcher
│           ├── print_catalog.h # one printer per layout (-l, -1, -m, -x, -C, default)
│           ├── format_info.h   # column width measurement
│           └── print_utils.h   # shared printing helpers
├── src/                        # implementations, mirroring include/
├── vector/
│   ├── include/
│   │   ├── file_data.h         # struct file_data: stat + lstat + names
│   │   └── file_vector.h       # growable vector of record pointers
│   └── src/
└── tests/
    ├── test.sh                 # compares my_ls output with GNU ls
    └── test_dir/               # fixture tree
```

## How It Works

Processing a command line goes through four stages:

1. **Flag parsing** (`src/flags/`). Each argument starting with `-` is a
   group of single-character flags. The parser looks each character up in
   a 256-entry table of setter functions and applies it to `struct ls_flags`.
   Unknown characters produce `ls: invalid option -- 'x'`.

2. **Scanning** (`src/file_scan.c`). Operands (or `.` when none) are
   `lstat`-ed and `stat`-ed into `struct file_data` records, which keep both
   results so the program can describe either the symlink or its target
   without touching the file system again. Directory contents are read
   with `readdir` and filtered by the `-a`/`-A` mode.

3. **Sorting** (`src/sorting.c`). Records live on the heap and the vector
   holds pointers to them, so sorting only swaps pointers. Ties on size or
   time are broken by name; `-r` negates the comparison.

4. **Printing** (`src/print/`). `ls_print_catalog()` dispatches on the
   output format. Every printer first measures the widest value of each
   field (`struct ls_format_info`) and then prints with padding, so columns
   line up exactly like GNU `ls`. The grid layouts (`-C`, `-x`, default on a
   terminal) search for the largest number of columns that fits the
   terminal width.

Recursion (`-R`) is handled in `src/ls.c`: after printing a directory it
is recorded in a *visited* list keyed by device and inode, and any
subdirectory already present there is skipped with
`ls: X: not listing already-listed directory`.

Memory ownership is simple: there is one container, `struct fdata_ptr_vector`,
holding pointers to heap-allocated records. A vector either *owns* its
records (released with `fdata_ptr_vector_destroy`) or is a *view* that
borrows them from an owning vector (released with `fdata_ptr_vector_free`).
Each record is created exactly once by `ls_get_file_info`; nothing is
copied. Views are always freed before the vector they point into.

## Documentation

All headers carry Doxygen comments describing every type, field and
function, and every source file starts with a summary of what it does.
A ready-made `Doxyfile` is included; to generate HTML documentation:

```bash
./build.sh docs       # or: make docs, or: doxygen Doxyfile
open docs/html/index.html
```

Doxygen must be installed (`brew install doxygen` on macOS,
`apt install doxygen` on Debian/Ubuntu). The generated `docs/` directory
is ignored by git.

## Build

Requirements: `gcc` (or `clang`), `make`, a POSIX system (Linux or macOS).

The quickest way is the helper script:

```bash
./build.sh            # build ./my_ls
./build.sh rebuild    # clean build from scratch
./build.sh clean      # remove build/ and the binary
./build.sh docs       # generate HTML documentation (needs doxygen)
./build.sh help       # list all commands
```

The script checks that the required tools are installed, builds in
parallel and can be run from any directory. Two environment variables
are honoured:

| Variable | Effect                                   |
|----------|------------------------------------------|
| `CC`     | compiler to use, e.g. `CC=clang ./build.sh` |
| `JOBS`   | number of parallel jobs (default: CPU count) |

Plain `make` works as well:

```bash
make          # build
make re       # rebuild
make clean    # remove build artifacts
make docs     # Doxygen HTML into docs/html
```

Both produce the `my_ls` binary in the project root.

## Usage

```bash
./my_ls [options] [files...]
```

Example

```bash
./my_ls -la /
```
![](/screenshots/my_ls_demonstration_2.png)

## Tests

The test suite (`tests/test.sh`) runs `my_ls` and GNU `ls` with the same
arguments and compares their output. It needs Linux: it relies on GNU
`ls` formatting and on the Linux version of `script`.

Run everything with one command from the project root:

```bash
./run_tests.sh            # Linux: runs locally; macOS: runs in Docker
./run_tests.sh --local    # force a local run
./run_tests.sh --docker   # force a run in an Ubuntu container
```

The script builds the project, copies the binary into `tests/` and runs
the suite with a timeout. On macOS it needs Docker Desktop to be running.
The Docker run works on a copy of the project, so nothing on the host
changes, and runs the tests as a regular user rather than root.

| Variable       | Effect                                         |
|----------------|------------------------------------------------|
| `TEST_TIMEOUT` | abort the suite after N seconds (default: 300) |
| `DOCKER_IMAGE` | image for Docker runs (default: `ubuntu:24.04`) |

The exit code is 0 when every test passes, so the script can be used in CI.

![](/screenshots/my_ls_test_4.png) 

## Memory Check (Valgrind)

```bash
valgrind --leak-check=full ./my_ls -la /
```

![](/screenshots/valgrind_memory_leak_test.png)

