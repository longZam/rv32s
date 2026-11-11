#include <executors.h>
#include <simulator.h>
#include <stdint.h>

static inline uint32_t __get_imm(uint32_t instruction)
{
    uint32_t result = 0;
    // 12
    result |= ((instruction >> 31) & 0x1) << 12;
    // 11
    result |= ((instruction >> 7) & 0x1) << 11;
    // 10:5
    result |= ((instruction >> 25) & 0x3f) << 5;
    // 4:1
    result |= ((instruction >> 8) & 0xf) << 1;

    // 산술 시프트로 부호 확장
    result = ((int32_t)(result << 19) >> 19);

    return result;
}

static inline uint32_t __get_rs1(uint32_t instruction)
{
    return (instruction >> 15) & 0x1f;
}

static inline uint32_t __get_rs2(uint32_t instruction)
{
    return (instruction >> 20) & 0x1f;
}

typedef int (*__comparer_fn)(uint32_t a, uint32_t b);

int __compare_greater_or_equal(uint32_t a, uint32_t b)
{
    return a >= b;
}

int __compare_less(uint32_t a, uint32_t b)
{
    return a < b;
}

int __compare_equal(uint32_t a, uint32_t b)
{
    return a == b;
}

int __compare_not_equal(uint32_t a, uint32_t b)
{
    return a != b;
}

static inline int __branch(simulator_t* simulator, __comparer_fn comparer)
{
    uint32_t rs1 = __get_rs1(simulator->instruction_register);
    uint32_t rs2 = __get_rs2(simulator->instruction_register);
    rs1 = simulator->general_perpose_registers[rs1];
    rs2 = simulator->general_perpose_registers[rs2];
    uint32_t next = 4;

    if (comparer(rs1, rs2))
        next = __get_imm(simulator->instruction_register);

    simulator->program_counter += next;

    return 1;
}

int __beq(simulator_t* simulator)
{
    return __branch(simulator, __compare_equal);
}

int __bge(simulator_t* simulator)
{
    return __branch(simulator, __compare_greater_or_equal);
}

int __bne(simulator_t* simulator)
{
    return __branch(simulator, __compare_not_equal);
}

int __blt(simulator_t* simulator)
{
    return __branch(simulator, __compare_less);
}
