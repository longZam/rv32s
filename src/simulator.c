#include <executors.h>
#include <simulator.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define __TEXT_SEGMENT_OFFSET 0x00040000
#define __DATA_SEGMENT_OFFSET 0x10000000

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
    // TODO: implement finding executor
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
    if (try_read_word_simulator(simulator, simulator->program_counter, &simulator->instruction_register))
        return 1;

    // decode
    __instruction_executor_fn executor = __decode(simulator->instruction_register);

    // execute
    return executor != NULL ? executor(simulator) : 1;
}

int try_read_word_simulator(simulator_t* simulator, uint32_t src_addr, uint32_t* dst_ptr)
{
    // check alignment
    if (src_addr % sizeof(uint32_t) != 0)
        return 1;

    // in data segment
    if (__DATA_SEGMENT_OFFSET < src_addr)
    {
        src_addr -= __DATA_SEGMENT_OFFSET;

        if (DATA_SEGMENT_SIZE < src_addr + sizeof(uint32_t))
            return 1;

        *dst_ptr = __read_uint32_little_endian(simulator->data_segment + src_addr);
        return 0;
    }
    // in text segment
    else if (__TEXT_SEGMENT_OFFSET < src_addr)
    {
        src_addr -= __TEXT_SEGMENT_OFFSET;

        if (TEXT_SEGMENT_SIZE < src_addr + sizeof(uint32_t))
            return 1;

        *dst_ptr = __read_uint32_little_endian(simulator->text_segment + src_addr);
        return 0;
    }
    else
        return 1;
}

int import_text_segment(simulator_t* simulator, const char* filename)
{
    FILE* fp = fopen(filename, "rb");

    if (fp == NULL)
        return 1;

    size_t total_read = 0;
    size_t bytes_read;

    // until EOF
    while ((bytes_read = fread(simulator->data_segment + total_read,
                               sizeof(uint8_t),
                               TEXT_SEGMENT_SIZE - total_read,
                               fp)) > 0)
    {
        total_read += bytes_read;

        if (TEXT_SEGMENT_SIZE <= total_read)
            break;
    }

    int failure = feof(fp);
    fclose(fp);
    return failure;
}

int import_data_segment(simulator_t* simulator, const char* filename)
{
    FILE* fp = fopen(filename, "rb");

    if (fp == NULL)
        return 1;

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

    int failure = feof(fp);
    fclose(fp);
    return failure;
}

int export_data_segment(simulator_t* simulator, const char* filename)
{
    FILE* fp = fopen(filename, "wb");

    if (fp == NULL)
        return 1;

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
    return total_written != DATA_SEGMENT_SIZE;
}
