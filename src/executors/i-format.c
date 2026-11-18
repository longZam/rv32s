#include <executors.h>
// 12 5 3 5 7

int __addi(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;

    // 11:7
    uint32_t rd = (inst >> 7) & 0x1F;

    // 19:15
    uint32_t rs1 = (inst >> 15) & 0x1F;

    // 31:20
    int32_t imm = (int32_t)inst >> 20;

    // x0이 아닐 때만
    if (rd != 0)
    {
        simulator->general_perpose_registers[rd] =
            simulator->general_perpose_registers[rs1] + (uint32_t)imm;
    }

    simulator->program_counter += 4;

    return 1;
}

int __andi(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;

    // 11:7
    uint32_t rd = (inst >> 7) & 0x1F;

    // 19:15
    uint32_t rs1 = (inst >> 15) & 0x1F;

    // 31:20
    int32_t imm = (int32_t)inst >> 20;

    // x0이 아닐 때만
    if (rd != 0)
    {
        simulator->general_perpose_registers[rd] =
            simulator->general_perpose_registers[rs1] & (uint32_t)imm;
    }

    simulator->program_counter += 4;

    return 1;
}

int __ori(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;

    // 11:7
    uint32_t rd = (inst >> 7) & 0x1F;
    
    // 19:15
    uint32_t rs1 = (inst >> 15) & 0x1F;
    
    // 31:20
    int32_t imm = (int32_t)inst >> 20;

    // x0이 아닐 때만
    if (rd != 0)
    {
        simulator->general_perpose_registers[rd] =
            simulator->general_perpose_registers[rs1] | (uint32_t)imm;
    }

    simulator->program_counter += 4;

    return 1;
}

int __lw(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;

    // 11:7
    uint32_t rd = (inst >> 7) & 0x1F;
    
    // 19:15
    uint32_t rs1 = (inst >> 15) & 0x1F;
    
    // 31:20
    int32_t imm = (int32_t)inst >> 20;

    // rs1 레지스터 값에 offset 만큼 더한 값
    uint32_t addr =
        simulator->general_perpose_registers[rs1] + (uint32_t)imm;

    // 주소가 4바이트인지 -> 즉 word인지 확인
    if (addr % 4 != 0)
        return 0;

    uint32_t value;

    // 메모리 읽기에 실패하면 0 반환
    if (!try_read_word_simulator(simulator, addr, &value))
        return 0;

    // x0이 아닐 경우만
    if (rd != 0)
        simulator->general_perpose_registers[rd] = value;

    simulator->program_counter += 4;

    return 1;
}
