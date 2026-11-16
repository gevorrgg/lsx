/**
 * @file ls.h
 * @brief Public entry point of the lsx program.
 *
 * The whole program is driven by a single function, ls(), which receives the
 * command-line arguments (without the program name), parses the flags, scans
 * the requested files and directories and prints them in the selected format.
 *
 * Typical usage from main():
 * @code
 * int main(int argc, const char *argv[])
 * {
 *     return ls(argc - 1, argv + 1) == LS_SUCCESS ? 0 : 1;
 * }
 * @endcode
 */

#ifndef LS_LS_H
#define LS_LS_H

#include <ls/error.h>

struct ls_flags;

/**
 * @brief Run the ls command on the given arguments.
 *
 * Processing pipeline:
 *  1. Parse flags (every argument starting with '-' before "--").
 *  2. Collect the file operands (or "." when none were given).
 *  3. Print non-directory operands first, then each directory operand,
 *     descending recursively when -R is set.
 *
 * Errors for individual operands (missing file, unreadable directory) are
 * reported to stderr and processing continues with the remaining operands.
 *
 * @param argc Number of arguments in @p argv (program name excluded).
 * @param argv Arguments: flags such as "-la" and file/directory operands.
 * @return LS_SUCCESS on success, otherwise the first fatal error encountered
 *         (unknown flag, allocation failure, ...).
 */
ls_error ls(int argc, const char *argv[]);

#endif //LS_LS_H
