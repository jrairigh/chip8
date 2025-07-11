#ifndef CHIP8_H
#define CHIP8_H

#define RUN_TESTS
#ifndef RUN_TESTS
void chip8_run();
#else
void chip8_run_tests();
#endif

#endif
