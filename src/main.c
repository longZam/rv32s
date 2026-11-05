#include <simulator.h>
#include <stdint.h>
#include <stdio.h>

int __run_simulator(simulator_t* simulator)
{
    int exit_code;
    while (!(exit_code = step_simulator(simulator)));
    return exit_code - 1;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;

    simulator_t* simulator = create_simulator();

    if (import_text_segment(simulator, argv[0]))
        return 1;

    if (import_data_segment(simulator, argv[1]))
        return 1;

    if (__run_simulator(simulator))
        return 1;

    if (export_data_segment(simulator, "data.out"))
        return 1;

    printf("PC 0x%08x invalid instruction 0x%08x\n", simulator->program_counter, simulator->instruction_register);
    destroy_simulator(simulator);
}
