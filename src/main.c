#include <simulator.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;

    simulator_t* simulator = create_simulator();

    if (import_text_segment(simulator, argv[0]))
        return 1;

    if (import_data_segment(simulator, argv[1]))
        return 1;

    for (int i = 0; i < 1000000 && !step_simulator(simulator); i++);

    if (export_data_segment(simulator, "data.out"))
        return 1;

    printf("PC 0x%08x invalid instruction 0x%08x\n", simulator->program_counter, simulator->instruction_register);
    destroy_simulator(simulator);
}
