#include "chip8.h"

int main(int argc, char *argv[])
{
    argc--; argv++;
    chip8_run(argv[0]);
    return 0;
}