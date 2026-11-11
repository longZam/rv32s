#include <simulator.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc < 3)
        return 1;

    simulator_t* simulator = create_simulator();

    if (!import_text_segment(simulator, argv[1]))
        return 1;

    if (!import_data_segment(simulator, argv[2]))
        return 1;

    while (step_simulator(simulator));

    if (!export_data_segment(simulator, "data.out"))
        return 1;

    printf("PC 0x%08x invalid instruction 0x%08x\n", simulator->program_counter, simulator->instruction_register);
    destroy_simulator(simulator);
}
