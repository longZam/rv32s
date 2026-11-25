#include <simulator.h>
#include <stdint.h>
#include <stdio.h>
#include <sysexits.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <text segment file> <data segment file>\n", argv[0]);
        return EX_USAGE;
    }

    simulator_t* simulator = create_simulator();

    if (simulator == NULL)
    {
        fprintf(stderr, "error: failed to create simulator (out of memory)\n");
        return EX_OSERR;
    }

    if (!import_text_segment(simulator, argv[1]))
    {
        fprintf(stderr,
                "error: failed to import text segment '%s' "
                "(unreadable file or larger than %d bytes)\n",
                argv[1], (int)TEXT_SEGMENT_SIZE);
        destroy_simulator(simulator);
        return EX_DATAERR;
    }

    if (!import_data_segment(simulator, argv[2]))
    {
        fprintf(stderr,
                "error: failed to import data segment '%s' "
                "(unreadable file or larger than %d bytes)\n",
                argv[2], (int)DATA_SEGMENT_SIZE);
        destroy_simulator(simulator);
        return EX_DATAERR;
    }

    while (step_simulator(simulator));

    if (!export_data_segment(simulator, "data.out"))
    {
        fprintf(stderr, "error: failed to write data segment to 'data.out'\n");
        destroy_simulator(simulator);
        return EX_CANTCREAT;
    }

    printf("PC 0x%08x invalid instruction 0x%08x\n", simulator->program_counter, simulator->instruction_register);
    destroy_simulator(simulator);

    return EX_OK;
}
