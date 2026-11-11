#include <executors.h>
#include <simulator.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __TEXT_SEGMENT_OFFSET 0x00040000
#define __DATA_SEGMENT_OFFSET 0x10000000
#define __EXECUTION_LIMIT 1000000

#pragma region inline

static inline uint32_t __get_opcode(uint32_t instruction)
{
    return instruction & 0x7F;
}

static inline uint32_t __get_funct3(uint32_t instruction)
{
    return (instruction >> 12) & 0x7;
}

static inline uint32_t __get_funct7(uint32_t instruction)
{
    return (instruction >> 25) & 0x7F;
}

static inline uint32_t __read_uint32_little_endian(const uint8_t* buffer)
{
    return (uint32_t)buffer[0] | ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 24);
}

#pragma endregion

typedef int (*__instruction_executor_fn)(simulator_t* simulator);

__instruction_executor_fn __decode(uint32_t instruction)
{
    uint32_t opcode = __get_opcode(instruction);
    uint32_t funct3 = __get_funct3(instruction);
    uint32_t funct7 = __get_funct7(instruction);

    switch (opcode)
    {
        case 0x03:
            switch (funct3)
            {
                case 0x2:
                    return __lw;
                default:
                    break;
            }
            break;

        case 0x23:
            switch (funct3)
            {
                case 0x2:
                    return __sw;
                default:
                    break;
            }
            break;

        case 0x13:
            switch (funct3)
            {
                case 0x0:
                    return __addi;
                case 0x6:
                    return __ori;
                case 0x7:
                    return __andi;
                default:
                    break;
            }
            break;

        case 0x33:
            switch (funct3)
            {
                case 0x0:
                    switch (funct7)
                    {
                        case 0x00:
                            return __add;
                        case 0x20:
                            return __sub;
                        default:
                            break;
                    }
                    break;

                case 0x6:
                    switch (funct7)
                    {
                        case 0x00:
                            return __or;
                        default:
                            break;
                    }
                    break;

                case 0x7:
                    switch (funct7)
                    {
                        case 0x00:
                            return __and;
                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }
            break;

        case 0x63:
            switch (funct3)
            {
                case 0x0:
                    return __beq;
                case 0x1:
                    return __bne;
                case 0x4:
                    return __blt;
                case 0x5:
                    return __bge;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    return NULL;
}

simulator_t* create_simulator()
{
    simulator_t* simulator = (simulator_t*)(malloc(sizeof(simulator_t)));
    simulator->program_counter = __TEXT_SEGMENT_OFFSET;
    simulator->instruction_register = 0;

    for (int i = 0; i < 32; ++i)
        simulator->general_perpose_registers[i] = 0;

    return simulator;
}

void destroy_simulator(simulator_t* ptr)
{
    free(ptr);
}

int step_simulator(simulator_t* simulator)
{
    // fetch
    if (!try_read_word_simulator(simulator, simulator->program_counter, &simulator->instruction_register))
        return 0;

    // decode
    __instruction_executor_fn executor = __decode(simulator->instruction_register);

    if (__EXECUTION_LIMIT <= simulator->execution_counter)
        return 0;

    simulator->execution_counter += 1;

    // execute
    return executor != NULL ? executor(simulator) : 0;
}

int try_read_word_simulator(simulator_t* simulator, uint32_t src_addr, uint32_t* dst_ptr)
{
    // in data segment
    if (__DATA_SEGMENT_OFFSET <= src_addr)
    {
        src_addr -= __DATA_SEGMENT_OFFSET;

        if (DATA_SEGMENT_SIZE < src_addr + sizeof(uint32_t))
            return 0;

        *dst_ptr = __read_uint32_little_endian(simulator->data_segment + src_addr);
        return 1;
    }
    // in text segment
    else if (__TEXT_SEGMENT_OFFSET <= src_addr)
    {
        src_addr -= __TEXT_SEGMENT_OFFSET;

        if (TEXT_SEGMENT_SIZE < src_addr + sizeof(uint32_t))
            return 0;

        *dst_ptr = __read_uint32_little_endian(simulator->text_segment + src_addr);
        return 1;
    }
    else
        return 0;
}

int import_text_segment(simulator_t* simulator, const char* filename)
{
    FILE* fp = fopen(filename, "rb");

    if (fp == NULL)
        return 0;

    size_t total_read = 0;
    size_t bytes_read;

    // until EOF
    while ((bytes_read = fread(simulator->text_segment + total_read,
                               sizeof(uint8_t),
                               TEXT_SEGMENT_SIZE - total_read,
                               fp)) > 0)
    {
        total_read += bytes_read;

        if (TEXT_SEGMENT_SIZE <= total_read)
            break;
    }

    int success = feof(fp);
    fclose(fp);
    return success;
}

int import_data_segment(simulator_t* simulator, const char* filename)
{
    memset(simulator->data_segment, 0, DATA_SEGMENT_SIZE);

    FILE* fp = fopen(filename, "rb");

    if (fp == NULL)
        return 0;

    size_t total_read = 0;
    size_t bytes_read;

    // until EOF
    while ((bytes_read = fread(simulator->data_segment + total_read,
                               sizeof(uint8_t),
                               DATA_SEGMENT_SIZE - total_read,
                               fp)) > 0)
    {
        total_read += bytes_read;

        if (DATA_SEGMENT_SIZE <= total_read)
            break;
    }

    int success = feof(fp);
    fclose(fp);
    return success;
}

int export_data_segment(simulator_t* simulator, const char* filename)
{
    FILE* fp = fopen(filename, "wb");

    if (fp == NULL)
        return 0;

    size_t total_written = 0;

    while (total_written < DATA_SEGMENT_SIZE)
    {
        size_t written = fwrite(simulator->data_segment + total_written,
                                sizeof(uint8_t),
                                DATA_SEGMENT_SIZE - total_written,
                                fp);

        total_written += written;

        if (written == 0)
            break;
    }

    fclose(fp);
    return total_written == DATA_SEGMENT_SIZE;
}
