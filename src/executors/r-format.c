#include <executors.h>

int __add(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;
    uint32_t rd = (inst >> 7) & 0x1F;
    uint32_t rs2 = (inst >> 20) & 0x1F;
    uint32_t rs1 = (inst >> 15) & 0x1F;

    // x0이 아닐 때만
    if (rd != 0)
    {
        simulator->general_perpose_registers[rd] =
            simulator->general_perpose_registers[rs1] + simulator->general_perpose_registers[rs2];
    }

    simulator->program_counter += 4;

    return 1;
}

int __and(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;
    uint32_t rd = (inst >> 7) & 0x1F;
    uint32_t rs2 = (inst >> 20) & 0x1F;
    uint32_t rs1 = (inst >> 15) & 0x1F;

    // x0이 아닐 때만
    if (rd != 0)
    {
        simulator->general_perpose_registers[rd] =
            simulator->general_perpose_registers[rs1] & simulator->general_perpose_registers[rs2];
    }

    simulator->program_counter += 4;

    return 1;
}

int __or(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;
    uint32_t rd = (inst >> 7) & 0x1F;
    uint32_t rs2 = (inst >> 20) & 0x1F;
    uint32_t rs1 = (inst >> 15) & 0x1F;

    // x0이 아닐 때만
    if (rd != 0)
    {
        simulator->general_perpose_registers[rd] =
            simulator->general_perpose_registers[rs1] | simulator->general_perpose_registers[rs2];
    }

    simulator->program_counter += 4;

    return 1;
}

int __sub(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;
    uint32_t rd = (inst >> 7) & 0x1F;
    uint32_t rs2 = (inst >> 20) & 0x1F;
    uint32_t rs1 = (inst >> 15) & 0x1F;

    // x0이 아닐 때만
    if (rd != 0)
    {
        simulator->general_perpose_registers[rd] =
            simulator->general_perpose_registers[rs1] - simulator->general_perpose_registers[rs2];
    }

    simulator->program_counter += 4;

    return 1;
}