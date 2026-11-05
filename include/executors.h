#pragma once
#include <simulator.h>
#include <stdint.h>

#pragma region R-format

int __add(simulator_t* simulator);

int __sub(simulator_t* simulator);

int __or(simulator_t* simulator);

int __and(simulator_t* simulator);

#pragma endregion

#pragma region I-format

int __addi(simulator_t* simulator);

int __ori(simulator_t* simulator);

int __andi(simulator_t* simulator);

int __lw(simulator_t* simulator);

#pragma endregion

#pragma region S-format

int __sw(simulator_t* simulator);

#pragma endregion

#pragma region U-format

int __lui(simulator_t* simulator);

#pragma endregion

#pragma region SB-format

int __beq(simulator_t* simulator);

int __bne(simulator_t* simulator);

int __blt(simulator_t* simulator);

int __bge(simulator_t* simulator);

#pragma endregion
