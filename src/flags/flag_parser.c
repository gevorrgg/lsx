/**
 * @file flag_parser.c
 * @brief Table-driven flag parser (see flag_parser.h).
 */
#include <lsx/flags/flag_parser.h>
#include <stdbool.h>
#include <lsx/flags/flags.h>
#include <lsx/error.h>
#include <string.h>

/** @brief A flag is supported when its slot in the table is non-NULL. */
static bool is_valid_flag(unsigned char flag, const flag_setter available_flags[])
{
        return available_flags[flag] != NULL;
}

lsx_error lsx_parse_flags(struct lsx_flags *flags, int argc, const char *args[], char *error_flag, const flag_setter flag_setters[])
{
        for (int i = 0; i < argc; i++)
        {
                const char *name = args[i];

                /* Operands are handled elsewhere (lsx_retrieve_files). */
                if (name[0] != '-')
                        continue;

                if (strcmp(name, "--") == 0)
                        break;  // end of flags

                /* "-la" is a group of single-character flags: apply each one
                 * in order, so later flags override earlier ones ("-lC" ends
                 * up in column format). */
                for (int j = 1; name[j]; j++)
                {
                        /* Cast to unsigned so bytes >= 0x80 index the table
                         * safely instead of going negative. */
                        unsigned char flag = (unsigned char) name[j];

                        if (is_valid_flag(flag, flag_setters))
                        {
                                flag_setters[flag](flags);
                        }
                        else
                        {
                                if (error_flag)
                                        *error_flag = (char) flag;

                                return LSX_UNKNOWN_FLAG_ERR;
                        }
                }
        }

        return LSX_SUCCESS;
}
