/**
 * @file lsx.h
 * @brief Public entry point of the lsx program.
 *
 * The whole program is driven by a single function, lsx(), which receives the
 * command-line arguments (without the program name), parses the flags, scans
 * the requested files and directories and prints them in the selected format.
 *
 * Typical usage from main():
 * @code
 * int main(int argc, const char *argv[])
 * {
 *     return lsx(argc - 1, argv + 1) == LSX_SUCCESS ? 0 : 1;
 * }
 * @endcode
 */

#ifndef LSX_H
#define LSX_H

#include <lsx/error.h>

struct lsx_flags;

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
 * @return LSX_SUCCESS on success, otherwise the first fatal error encountered
 *         (unknown flag, allocation failure, ...).
 */
lsx_error lsx(int argc, const char *argv[]);

#endif //LSX_H
