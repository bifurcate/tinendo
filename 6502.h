#ifndef __6502_H
#define __6502_H

#include <stdint.h>
#include "nesmem.h"

#define CPU_6502_SIGNAL__RESET  ( 1 << 0 )
#define CPU_6502_SIGNAL__NMI    ( 1 << 1 )
#define CPU_6502_SIGNAL__IRQ    ( 1 << 2 )

typedef unsigned int cpu6502Signal;

struct cpu6502 {

    unsigned int clk;  // tick counter

    uint16_t PC;  // Program Counter
    uint8_t  SP;  // Stack Pointer
    uint8_t  A;   // Accumulator
    uint8_t  X;   // Index Register X 
    uint8_t  Y;   // Index Register Y
    uint8_t  P;   // Processor Status

    uint8_t opcode;   // opcode
    uint8_t o1;   // operand 1
    uint8_t o2;   // operand 2

    struct nesMemoryMap *mm;

};

void cpu6502Init( struct cpu6502 *cpu );
void cpu6502Step( struct cpu6502 *cpu, cpu6502Signal sig );
void cpu6502PrintDebugInfo( struct cpu6502 *cpu);
void cpu6502PrintInstruction( struct cpu6502 *cpu );

#endif /* __6502_H */

