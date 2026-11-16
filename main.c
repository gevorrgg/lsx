/**
 * @file main.c
 * @brief Program entry point: strips the program name and hands the
 *        remaining arguments to lsx().
 */
#include <lsx.h>

int main(const int argc, const char* argv[])
{
	/* argv[0] is the program name; lsx() only wants flags and operands. */
	return lsx(argc - 1, argv + 1) == LSX_SUCCESS ? 0 : 1;
}
