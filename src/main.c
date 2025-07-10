#include "chip8.h"

int main(int argc, char *argv[])
{
#ifdef RUN_TESTS
    chip8_run_tests();
#else
    chip8_run();
#endif

    return 0;
}