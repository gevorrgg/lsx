/**
 * @file flag_setters.h
 * @brief One setter function per supported flag.
 *
 * Each `set_X` applies the effect of `-X` to a struct lsx_flags. The setters
 * are collected into a 256-entry table by lsx_get_flag_setters() so that the
 * parser can dispatch on the flag character directly.
 *
 * Supported flags and their effect:
 *
 * | Flag | Effect                                                      |
 * |------|-------------------------------------------------------------|
 * | -l   | long listing format                                         |
 * | -R   | list subdirectories recursively                             |
 * | -a   | show all entries, including "." and ".."                    |
 * | -A   | show hidden entries except "." and ".."                     |
 * | -h   | human-readable sizes (1K, 2.5M, ...)                        |
 * | -d   | list directories themselves, not their contents             |
 * | -S   | sort by size, largest first                                 |
 * | -t   | sort by time (modification unless -u/-c)                    |
 * | -r   | reverse sort order                                          |
 * | -U   | do not sort (directory order)                               |
 * | -f   | same as -aU                                                 |
 * | -1   | one entry per line                                          |
 * | -m   | comma-separated list filling the line width                 |
 * | -x   | columns, filled row by row                                  |
 * | -C   | columns, filled column by column                            |
 * | -i   | print inode number                                          |
 * | -n   | long format with numeric UID/GID                            |
 * | -g   | long format without the owner column                        |
 * | -o   | long format without the group column                        |
 * | -F   | append indicator (/, @, *) to entries                       |
 * | -p   | append "/" to directories                                   |
 * | -s   | print allocated size in 1K blocks                           |
 * | -u   | use access time for -t and -l                               |
 * | -c   | use status-change time for -t and -l                        |
 * | -L   | follow symbolic links                                       |
 */

#ifndef LSX_FLAG_SETTERS_H
#define LSX_FLAG_SETTERS_H

#include <lsx/flags/flags.h>

/** @brief Function that applies one flag to a struct lsx_flags. */
typedef void (*flag_setter) (struct lsx_flags *f);

void set_l(struct lsx_flags* f); /**< -l: long format. */
void set_R(struct lsx_flags* f); /**< -R: recursive. */
void set_a(struct lsx_flags* f); /**< -a: show all, including "." and "..". */
void set_A(struct lsx_flags* f); /**< -A: show hidden except "." and "..". */
void set_h(struct lsx_flags* f); /**< -h: human-readable sizes. */
void set_d(struct lsx_flags* f); /**< -d: do not enter directories. */
void set_S(struct lsx_flags* f); /**< -S: sort by size. */
void set_t(struct lsx_flags* f); /**< -t: sort by time. */
void set_r(struct lsx_flags* f); /**< -r: reverse order. */
void set_U(struct lsx_flags* f); /**< -U: no sorting. */
void set_f(struct lsx_flags* f); /**< -f: -a and -U together. */
void set_1(struct lsx_flags* f); /**< -1: one entry per line. */
void set_m(struct lsx_flags* f); /**< -m: comma-separated. */
void set_x(struct lsx_flags* f); /**< -x: horizontal columns. */
void set_C(struct lsx_flags* f); /**< -C: vertical columns. */
void set_i(struct lsx_flags* f); /**< -i: show inode. */
void set_n(struct lsx_flags* f); /**< -n: long format, numeric ids. */
void set_g(struct lsx_flags* f); /**< -g: long format, no owner. */
void set_o(struct lsx_flags* f); /**< -o: long format, no group. */
void set_F(struct lsx_flags* f); /**< -F: classify all entries. */
void set_p(struct lsx_flags* f); /**< -p: slash after directories. */
void set_s(struct lsx_flags* f); /**< -s: show block counts. */
void set_u(struct lsx_flags* f); /**< -u: use access time. */
void set_c(struct lsx_flags* f); /**< -c: use change time. */
void set_L(struct lsx_flags* f); /**< -L: follow symlinks. */

/**
 * @brief Get the flag dispatch table.
 *
 * @return Pointer to a static array of 256 setters indexed by the flag
 *         character. Entries for unsupported characters are NULL.
 */
const flag_setter * lsx_get_flag_setters();



#endif
