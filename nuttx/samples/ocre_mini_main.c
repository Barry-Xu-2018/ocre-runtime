#include <ocre/platform/config.h>
#include <ocre/platform/workdir.h>

/* The NuttX build renames main() of this file to ocre_mini_main() */

#pragma push_macro("main")
#undef main
#define main ocre_mini_sample_main
#include "../../src/samples/mini/main.c"
#undef main
#pragma pop_macro("main")

int main(int argc, char *argv[])
{
	if (ocre_check_working_directory(CONFIG_OCRE_DEFAULT_WORKING_DIRECTORY)) {
		return 1;
	}

	return ocre_mini_sample_main(argc, argv);
}
