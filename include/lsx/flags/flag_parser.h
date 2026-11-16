/**
 * @file flag_parser.h
 * @brief Command-line flag parsing.
 *
 * Flags are parsed with a lookup table indexed by the flag character:
 * `flag_setters['l']` is the function that applies -l, and so on. This keeps
 * the parser itself free of any knowledge about individual options.
 */

#ifndef LSX_FLAG_PARSER_H
#define LSX_FLAG_PARSER_H

#include <stdbool.h>

#include <lsx/error.h>
#include <lsx/flags/flags.h>

/** @brief Function that applies one flag to a struct lsx_flags. */
typedef void (*flag_setter)(struct lsx_flags* f);

/**
 * @brief Apply every flag found in @p args to @p flags.
 *
 * Each argument starting with '-' is treated as a group of single-character
 * flags ("-la" == "-l -a"). Parsing stops at "--". Arguments not starting
 * with '-' are ignored here; they are collected later by lsx_retrieve_files().
 *
 * @param flags        Flag structure, already initialised with lsx_flags_init().
 * @param argc         Argument count.
 * @param args         Arguments.
 * @param error_flag   Optional output: the offending character when an
 *                     unknown flag is met.
 * @param flag_setters Table of 256 setters indexed by character; NULL entries
 *                     mark unsupported flags (see lsx_get_flag_setters()).
 * @return LSX_SUCCESS or LSX_UNKNOWN_FLAG_ERR.
 */
lsx_error lsx_parse_flags(struct lsx_flags *flags, int argc, const char *args[], char *error_flag, const flag_setter flag_setters[]);

#endif
