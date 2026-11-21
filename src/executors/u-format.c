#include <executors.h>

int __lui(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;
    uint32_t rd = (inst >> 7) & 0x1F;
    int32_t imm = (int32_t)inst & 0xFFFFF000;

    // x0이 아닐 때만
    if (rd != 0)
    {
        simulator->general_perpose_registers[rd] = (uint32_t)imm;
    }

    simulator->program_counter += 4;

    return 1;
}