#include "codes.h"

#ifdef RUN_TESTS
#define BEGIN_TEST(name) { \
    chip8_initialize(); \
    total_tests++; \
    const char* test_name = name; \
    uint16_t program[] = {

#define RUN_TEST }; \
    memcpy(&s_chip8.ram[PROGRAM_START], program, sizeof(program)); \
    while((*(uint16_t*)&s_chip8.ram[s_chip8.pc]) != 0x0000) \
        chip8_vm_run(); \
    bool passed = true;
    
#define ASSERT_REG(reg, value) passed = passed && (s_chip8.v[reg] == value);
#define ASSERT_SP(value) passed = passed && (s_chip8.sp == value);
#define ASSERT_PC(value) passed = passed && (s_chip8.pc == value);
#define ASSERT_INDEX(value) passed = passed && (s_chip8.index == value);
#define ASSERT_STACK(level, value) passed = passed && (s_chip8.stack[level] == value);
#define ASSERT_DT(value) passed = passed && (s_chip8.delay_timer == value);
#define ASSERT_ST(value) passed = passed && (s_chip8.sound_timer == value);
#define ASSERT_MEM(index, value) passed = passed && (s_chip8.ram[index] == value);

#define END_TEST \
    if(passed) { \
        passed_tests++; \
    } \
    printf("Test %-35s %s\n", test_name, passed ? "PASSED" : "FAILED");}
#endif