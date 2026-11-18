#include <executors.h>
// 7 5 5 3 5 7
// imm[11:5] rs2 rs1 func3 imm[4:0] opcode

int __sw(simulator_t* simulator)
{
    uint32_t inst = simulator->instruction_register;

    // 19:15
    uint32_t rs1 = (inst >> 15) & 0x1F;
    
    // 24:20
    uint32_t rs2 = (inst >> 20) & 0x1F;

    // imm[4:0], 11:7
    uint32_t imm_low = (inst >> 7) & 0x1F;
    
    // imm[11:5], 31:25
    uint32_t imm_high = (inst >> 25) & 0x7F;

    // 즉시값 하나로 합치기
    uint32_t imm = (imm_high << 5) | imm_low;

    // 즉시값이 음수일 때 (0x800 = 1000 0000 0000) 부호 확장
    if (imm & 0x800)
        imm |= 0xFFFFF000;

    // 메모리 주소는 rs1 레지스터 값에 offset 더한 값
    uint32_t addr =
        simulator->general_perpose_registers[rs1] + imm;

    // word인지 확인
    if (addr % 4 != 0)
        return 0;

    // 저장할 값 
    uint32_t value = simulator->general_perpose_registers[rs2];

    // 메모리에 저장 시도
    if (!try_write_word_simulator(simulator, value, addr))
        return 0;

    simulator->program_counter += 4;
    
    return 1;
}
