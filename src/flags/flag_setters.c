/**
 * @file flag_setters.c
 * @brief Effect of each flag on struct lsx_flags (see flag_setters.h).
 *
 * Setters are intentionally tiny and order-dependent: the parser applies
 * them left to right, so "-lC" ends in column format while "-Cl" ends in
 * long format, just like GNU ls.
 */
#include <lsx/flags/flag_setters.h>

/* Output format. */
void set_l(struct lsx_flags* f) { f->format =  FORMAT_LONG; }
void set_1(struct lsx_flags* f) { f->format = FORMAT_ONE_PER_LINE; }
void set_m(struct lsx_flags* f) { f->format = FORMAT_COMMAS; }
void set_x(struct lsx_flags* f) { f->format = FORMAT_HORIZONTAL; }
void set_C(struct lsx_flags* f) { f->format = FORMAT_COLUMNS; }

/* Long-format variants: all imply -l and drop or change one column. */
void set_n(struct lsx_flags* f) { f->format=FORMAT_LONG; f->print_owner_name=false; f->print_group_name=false; }
void set_g(struct lsx_flags* f) { f->format=FORMAT_LONG; f->show_owner=false; f->show_group=true; }
void set_o(struct lsx_flags* f) { f->format=FORMAT_LONG; f->show_owner=true; f->show_group=false; }

/* Traversal. */
void set_R(struct lsx_flags* f) { f->is_recursive = true; }
void set_d(struct lsx_flags* f) { f->enter_dir = false; }
void set_L(struct lsx_flags* f) { f->follow_symlinks = true; }

/* Hidden files. */
void set_a(struct lsx_flags* f) { f->hidden = SHOW_ALL; }
void set_A(struct lsx_flags* f) { f->hidden = SHOW_ALMOST; }

/* Sorting. -f is a shorthand for -aU. */
void set_S(struct lsx_flags* f) { f->sort = SORT_SIZE; }
void set_t(struct lsx_flags* f) { f->sort = SORT_TIME; }
void set_r(struct lsx_flags* f) { f->sort_reversed = true; }
void set_U(struct lsx_flags* f) { f->sort = SORT_NONE; }
void set_f(struct lsx_flags* f) { f->hidden = SHOW_ALL; f->sort = SORT_NONE; }

/* Which timestamp -t and -l use. */
void set_u(struct lsx_flags* f) { f->time_type=TIME_ACCESS; }
void set_c(struct lsx_flags* f) { f->time_type=TIME_CHANGE; }

/* Extra per-entry fields. */
void set_h(struct lsx_flags* f) { f->is_human_readable = true; }
void set_i(struct lsx_flags* f) { f->show_inode = true; }
void set_s(struct lsx_flags* f) { f->show_blocks=true; }

/* Type indicators. */
void set_F(struct lsx_flags* f) { f->symbols_mode=SYMBOLS_ALL; }
void set_p(struct lsx_flags* f) { f->symbols_mode=SYMBOLS_DIRECTORIES; }

const flag_setter *lsx_get_flag_setters()
{
    /* Designated initialisers leave every other slot NULL, which the parser
     * treats as "unknown flag". Adding a flag = adding one line here plus
     * a setter above. */
    static const flag_setter flag_setters[256] = {
        ['l'] = set_l,
        ['R'] = set_R,
        ['a'] = set_a,
        ['A'] = set_A,
        ['h'] = set_h,
        ['d'] = set_d,
        ['S'] = set_S,
        ['t'] = set_t,
        ['r'] = set_r,
        ['U'] = set_U,
        ['f'] = set_f,
        ['1'] = set_1,
        ['m'] = set_m,
        ['x'] = set_x,
        ['C'] = set_C,
        ['i'] = set_i,
        ['n'] = set_n,
        ['g'] = set_g,
        ['o'] = set_o,
        ['F'] = set_F,
        ['p'] = set_p,
        ['s'] = set_s,
        ['u'] = set_u,
        ['c'] = set_c,
        ['L'] = set_L,
    };

    return flag_setters;
}
