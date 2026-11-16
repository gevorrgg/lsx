/**
 * @file flags.h
 * @brief Parsed command-line options.
 *
 * struct lsx_flags is filled once at start-up and then passed read-only to
 * every module. It describes *what* the user asked for; the printing and
 * sorting code decide *how* to honour it.
 */

#ifndef LSX_FLAGS_H
#define LSX_FLAGS_H

#include <stdbool.h>

/** @brief Sort key for directory entries. */
typedef enum {
        SORT_NAME,   /**< Alphabetical by name (default). */
        SORT_NONE,   /**< Keep directory order (-U, -f). */
        SORT_TIME,   /**< By timestamp selected with TimeType (-t). */
        SORT_SIZE    /**< By size, largest first (-S). */
} SortType;

/** @brief Output layout. */
typedef enum {
        FORMAT_STANDARD,      /**< Columns when stdout is a terminal, else one per line. */
		FORMAT_LONG,          /**< Long format (-l, -n, -g, -o). */
        FORMAT_COLUMNS,       /**< Columns filled top to bottom (-C). */
        FORMAT_ONE_PER_LINE,  /**< One entry per line (-1). */
        FORMAT_COMMAS,        /**< Comma-separated, wrapped at line width (-m). */
        FORMAT_HORIZONTAL,    /**< Columns filled left to right (-x). */
		FORMAT_NONE           /**< Unused sentinel. */
} OutputFormat;

/** @brief Which timestamp -t sorts by and -l displays. */
typedef enum {
        TIME_MODIFICATION,  /**< st_mtime (default). */
        TIME_ACCESS,        /**< st_atime (-u). */
        TIME_CHANGE         /**< st_ctime (-c). */
} TimeType;

/** @brief Handling of entries whose name starts with '.'. */
typedef enum {
        HIDE_DEFAULT,  /**< Hide all dot-files. */
        SHOW_ALL,      /**< Show everything, including "." and ".." (-a). */
        SHOW_ALMOST    /**< Show dot-files but not "." and ".." (-A). */
} HiddenMode;

/** @brief Type indicator appended after file names. */
typedef enum {
        SYMBOLS_NONE,         /**< No indicator. */
        SYMBOLS_DIRECTORIES,  /**< "/" after directories only (-p). */
        SYMBOLS_ALL           /**< "/", "@", "*" as appropriate (-F). */
} AddSymbolsMode;

/**
 * @brief All options understood by lsx.
 *
 * Initialise with lsx_flags_init() before parsing; see flag_setters.h for the
 * mapping between flag characters and these fields.
 */
struct lsx_flags
{
        bool is_recursive;       /**< -R: descend into subdirectories. */
        bool is_human_readable;  /**< -h: print sizes as 1K, 2.5M, ... */
        bool show_inode;         /**< -i: print inode number before each entry. */
        bool sort_reversed;      /**< -r: reverse the sort order. */
        bool show_owner;         /**< Long format: print the owner column (false with -g). */
        bool show_group;         /**< Long format: print the group column (false with -o). */
        bool follow_symlinks;    /**< -L: use the link target's metadata. */
        bool show_blocks;        /**< -s: print allocated blocks before each entry. */
        bool enter_dir;          /**< List directory contents (false with -d). */
		bool print_owner_name;   /**< Print owner as a name rather than UID (false with -n). */
		bool print_group_name;   /**< Print group as a name rather than GID (false with -n). */

        SortType sort;               /**< Sort key. */
        OutputFormat format;         /**< Output layout. */
        TimeType time_type;          /**< Timestamp used for sorting/display. */
        HiddenMode hidden;           /**< Dot-file visibility. */
        AddSymbolsMode symbols_mode; /**< Type indicator mode. */
};

/**
 * @brief Reset @p self to the defaults of plain `ls` with no options.
 *
 * Defaults: sort by name, standard format, modification time, hide
 * dot-files, no indicators, enter directories, show owner and group names.
 */
void lsx_flags_init(struct lsx_flags *self);

#endif
