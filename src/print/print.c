/**
 * @file print.c
 * @brief Dispatch from `flags->format` to the matching printer.
 */
#include <lsx/print/print.h>
#include <lsx/print/print_catalog.h>
#include <file_data.h>
#include <lsx/flags/flags.h>
#include <stdio.h>
#include <assert.h>

void lsx_print_catalog (const struct fdata_ptr_vector *file_ptrs,
                       const struct lsx_flags *flags)
{
    switch (flags->format)
    {
        case FORMAT_COMMAS:
            lsx_print_commas(file_ptrs, flags);

            break;
        case FORMAT_LONG:
            lsx_print_long(file_ptrs, flags);

            break;
        case FORMAT_ONE_PER_LINE:
            lsx_print_one_per_line(file_ptrs, flags);

            break;
        case FORMAT_HORIZONTAL:
            lsx_print_horizontal(file_ptrs, flags);

            break;
        case FORMAT_COLUMNS:
            lsx_print_columns(file_ptrs, flags);

            break;
        case FORMAT_STANDARD:
            lsx_print_standard(file_ptrs, flags);

            break;
        default:
            /* Every OutputFormat value must have a case above; reaching
             * here means a new format was added without a printer. */
			printf("Format: %d\n", flags->format);
            assert(false);
    }
}
