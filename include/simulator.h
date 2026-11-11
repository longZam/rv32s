#pragma once
#include <stdint.h>

#define TEXT_SEGMENT_SIZE 1024
#define DATA_SEGMENT_SIZE 1024

typedef struct
{
    int32_t execution_counter;
    uint32_t program_counter;
    uint32_t instruction_register;
    uint32_t general_perpose_registers[32];

    uint8_t text_segment[TEXT_SEGMENT_SIZE];
    uint8_t data_segment[DATA_SEGMENT_SIZE];
} simulator_t;

simulator_t* create_simulator();

void destroy_simulator(simulator_t* ptr);

int step_simulator(simulator_t* simulator);

int try_read_word_simulator(simulator_t* simulator, uint32_t src_addr, uint32_t* dst_ptr);

int try_write_word_simulator(simulator_t* simulator, uint32_t src_word, uint32_t dst_addr);

int import_text_segment(simulator_t* simulator, const char* filename);

int import_data_segment(simulator_t* simulator, const char* filename);

int export_data_segment(simulator_t* simulator, const char* filename);
