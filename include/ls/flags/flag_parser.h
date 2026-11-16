/**
 * @file flag_parser.h
 * @brief Command-line flag parsing.
 *
 * Flags are parsed with a lookup table indexed by the flag character:
 * `flag_setters['l']` is the function that applies -l, and so on. This keeps
 * the parser itself free of any knowledge about individual options.
 */

#ifndef FLAG_PARSER_H
#define FLAG_PARSER_H

#include <stdbool.h>

#include <ls/error.h>
#include <ls/flags/flags.h>

/** @brief Function that applies one flag to a struct ls_flags. */
typedef void (*flag_setter)(struct ls_flags* f);

/**
 * @brief Apply every flag found in @p args to @p flags.
 *
 * Each argument starting with '-' is treated as a group of single-character
 * flags ("-la" == "-l -a"). Parsing stops at "--". Arguments not starting
 * with '-' are ignored here; they are collected later by ls_retrieve_files().
 *
 * @param flags        Flag structure, already initialised with ls_flags_init().
 * @param argc         Argument count.
 * @param args         Arguments.
 * @param error_flag   Optional output: the offending character when an
 *                     unknown flag is met.
 * @param flag_setters Table of 256 setters indexed by character; NULL entries
 *                     mark unsupported flags (see ls_get_flag_setters()).
 * @return LS_SUCCESS or LS_UNKNOWN_FLAG_ERR.
 */
ls_error ls_parse_flags(struct ls_flags *flags, int argc, const char *args[], char *error_flag, const flag_setter flag_setters[]);

#endif
