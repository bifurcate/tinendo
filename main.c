#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "6502.h"
#include "nesmem.h"

int main(int argc, char *argv[]) {

    int i; int r; int c; int n;
    int initial_steps;
    struct nesMemoryMap testMM;
    struct cpu6502 cpu = {0};
    cpu6502Signal sig = 0;

    nesMemoryMapTestInit( &testMM );
    r = nesMemLoadINES( &testMM, argv[1] );

    if (r<0) {
        exit(-r);
    }

    initial_steps = atoi(argv[2]);

    cpu6502Init(&cpu);
    cpu.mm = &testMM;
    cpu.PC = 0x8000;

    for (i=0;i<6;i++) printf("[ %04x = $%02" PRIx8 " ] ", 0xfffa + i , cpu.mm->mem[0xfffa + i]);

    cpu6502PrintDebugInfo(&cpu);

    for (i=0; i<initial_steps; i++) {
        cpu6502Step(&cpu,sig);
    }

    while (1) {

        c = getchar();
        switch(c) {
            case '1':
                n = 16;
                sig = 0;
                for (i=0; i<n; i++) {
                    cpu6502Step(&cpu,sig);
                    cpu6502PrintInstruction(&cpu);
                    cpu6502PrintDebugInfo(&cpu);
                }
                break;
            case '2':
                n = 256;
                sig = 0;
                for (i=0; i<n; i++) {
                    cpu6502Step(&cpu,sig);
                    cpu6502PrintInstruction(&cpu);
                    cpu6502PrintDebugInfo(&cpu);
                }
                break;
            default:
                n = 1;
                sig = 0;
                for (i=0; i<n; i++) {
                    cpu6502Step(&cpu,sig);
                    cpu6502PrintInstruction(&cpu);
                    cpu6502PrintDebugInfo(&cpu);
                }
                break;
            case 'i':
                n = 1;
                sig = CPU_6502_SIGNAL__NMI;
                for (i=0; i<n; i++) {
                    cpu6502Step(&cpu,sig);
                    cpu6502PrintInstruction(&cpu);
                    cpu6502PrintDebugInfo(&cpu);
                }
                break;
            case 'q':
                exit(0);
                break;
            case 'n':
                ppuMemDumpNameTable(0);
                break;
            case 'm':
                ppuMemDumpNameTable(1);
                break;
            case 'z':
                cpuMemDumpPage(cpu.mm,0);
                break;
            case 's':
                cpuDumpStack(&cpu);
                break;
            case 'x':
                cpuMemDumpNonZero(cpu.mm);
                break;
    
        }
    }

    return 0;
}
