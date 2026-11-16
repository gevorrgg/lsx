/**
 * @file main.c
 * @brief Program entry point: strips the program name and hands the
 *        remaining arguments to ls().
 */
#include <ls.h>

int main(const int argc, const char* argv[])
{
	/* argv[0] is the program name; ls() only wants flags and operands. */
	return ls(argc - 1, argv + 1) == LS_SUCCESS ? 0 : 1;
}
