#include <stdio.h>
#include <inttypes.h>

#define ROM_SIZE 16384

enum mode_t {
    MODE_ABSOLUTE = 1,
    MODE_ABSOLUTE_X,
    MODE_ABSOLUTE_Y,
    MODE_ACCUMULATOR,
    MODE_IMMEDIATE,
    MODE_IMPLIED,
    MODE_INDEXED_INDIRECT,
    MODE_INDIRECT,
    MODE_INDIRECT_INDEXED,
    MODE_RELATIVE,
    MODE_ZERO_PAGE,
    MODE_ZERO_PAGE_X,
    MODE_ZERO_PAGE_Y
}

struct instruction {
    char name[3];
    unsigned int mode;
    unsigned int size;
    unsigned int cycles;
    unsigned int pageCycles;
};

struct instruction instructionSet[256] = {
    { "BRK", 6, 1, 7, 0 },
    { "ORA", 7, 2, 6, 0 },
    { "KIL", 6, 0, 2, 0 },
    { "SLO", 7, 0, 8, 0 },
    { "NOP", 11, 2, 3, 0 },
    { "ORA", 11, 2, 3, 0 },
    { "ASL", 11, 2, 5, 0 },
    { "SLO", 11, 0, 5, 0 },
    { "PHP", 6, 1, 3, 0 },
    { "ORA", 5, 2, 2, 0 },
    { "ASL", 4, 1, 2, 0 },
    { "ANC", 5, 0, 2, 0 },
    { "NOP", 1, 3, 4, 0 },
    { "ORA", 1, 3, 4, 0 },
    { "ASL", 1, 3, 6, 0 },
    { "SLO", 1, 0, 6, 0 },
    { "BPL", 10, 2, 2, 1 },
    { "ORA", 9, 2, 5, 1 },
    { "KIL", 6, 0, 2, 0 },
    { "SLO", 9, 0, 8, 0 },
    { "NOP", 12, 2, 4, 0 },
    { "ORA", 12, 2, 4, 0 },
    { "ASL", 12, 2, 6, 0 },
    { "SLO", 12, 0, 6, 0 },
    { "CLC", 6, 1, 2, 0 },
    { "ORA", 3, 3, 4, 1 },
    { "NOP", 6, 1, 2, 0 },
    { "SLO", 3, 0, 7, 0 },
    { "NOP", 2, 3, 4, 1 },
    { "ORA", 2, 3, 4, 1 },
    { "ASL", 2, 3, 7, 0 },
    { "SLO", 2, 0, 7, 0 },
    { "JSR", 1, 3, 6, 0 },
    { "AND", 7, 2, 6, 0 },
    { "KIL", 6, 0, 2, 0 },
    { "RLA", 7, 0, 8, 0 },
    { "BIT", 11, 2, 3, 0 },
    { "AND", 11, 2, 3, 0 },
    { "ROL", 11, 2, 5, 0 },
    { "RLA", 11, 0, 5, 0 },
    { "PLP", 6, 1, 4, 0 },
    { "AND", 5, 2, 2, 0 },
    { "ROL", 4, 1, 2, 0 },
    { "ANC", 5, 0, 2, 0 },
    { "BIT", 1, 3, 4, 0 },
    { "AND", 1, 3, 4, 0 },
    { "ROL", 1, 3, 6, 0 },
    { "RLA", 1, 0, 6, 0 },
    { "BMI", 10, 2, 2, 1 },
    { "AND", 9, 2, 5, 1 },
    { "KIL", 6, 0, 2, 0 },
    { "RLA", 9, 0, 8, 0 },
    { "NOP", 12, 2, 4, 0 },
    { "AND", 12, 2, 4, 0 },
    { "ROL", 12, 2, 6, 0 },
    { "RLA", 12, 0, 6, 0 },
    { "SEC", 6, 1, 2, 0 },
    { "AND", 3, 3, 4, 1 },
    { "NOP", 6, 1, 2, 0 },
    { "RLA", 3, 0, 7, 0 },
    { "NOP", 2, 3, 4, 1 },
    { "AND", 2, 3, 4, 1 },
    { "ROL", 2, 3, 7, 0 },
    { "RLA", 2, 0, 7, 0 },
    { "RTI", 6, 1, 6, 0 },
    { "EOR", 7, 2, 6, 0 },
    { "KIL", 6, 0, 2, 0 },
    { "SRE", 7, 0, 8, 0 },
    { "NOP", 11, 2, 3, 0 },
    { "EOR", 11, 2, 3, 0 },
    { "LSR", 11, 2, 5, 0 },
    { "SRE", 11, 0, 5, 0 },
    { "PHA", 6, 1, 3, 0 },
    { "EOR", 5, 2, 2, 0 },
    { "LSR", 4, 1, 2, 0 },
    { "ALR", 5, 0, 2, 0 },
    { "JMP", 1, 3, 3, 0 },
    { "EOR", 1, 3, 4, 0 },
    { "LSR", 1, 3, 6, 0 },
    { "SRE", 1, 0, 6, 0 },
    { "BVC", 10, 2, 2, 1 },
    { "EOR", 9, 2, 5, 1 },
    { "KIL", 6, 0, 2, 0 },
    { "SRE", 9, 0, 8, 0 },
    { "NOP", 12, 2, 4, 0 },
    { "EOR", 12, 2, 4, 0 },
    { "LSR", 12, 2, 6, 0 },
    { "SRE", 12, 0, 6, 0 },
    { "CLI", 6, 1, 2, 0 },
    { "EOR", 3, 3, 4, 1 },
    { "NOP", 6, 1, 2, 0 },
    { "SRE", 3, 0, 7, 0 },
    { "NOP", 2, 3, 4, 1 },
    { "EOR", 2, 3, 4, 1 },
    { "LSR", 2, 3, 7, 0 },
    { "SRE", 2, 0, 7, 0 },
    { "RTS", 6, 1, 6, 0 },
    { "ADC", 7, 2, 6, 0 },
    { "KIL", 6, 0, 2, 0 },
    { "RRA", 7, 0, 8, 0 },
    { "NOP", 11, 2, 3, 0 },
    { "ADC", 11, 2, 3, 0 },
    { "ROR", 11, 2, 5, 0 },
    { "RRA", 11, 0, 5, 0 },
    { "PLA", 6, 1, 4, 0 },
    { "ADC", 5, 2, 2, 0 },
    { "ROR", 4, 1, 2, 0 },
    { "ARR", 5, 0, 2, 0 },
    { "JMP", 8, 3, 5, 0 },
    { "ADC", 1, 3, 4, 0 },
    { "ROR", 1, 3, 6, 0 },
    { "RRA", 1, 0, 6, 0 },
    { "BVS", 10, 2, 2, 1 },
    { "ADC", 9, 2, 5, 1 },
    { "KIL", 6, 0, 2, 0 },
    { "RRA", 9, 0, 8, 0 },
    { "NOP", 12, 2, 4, 0 },
    { "ADC", 12, 2, 4, 0 },
    { "ROR", 12, 2, 6, 0 },
    { "RRA", 12, 0, 6, 0 },
    { "SEI", 6, 1, 2, 0 },
    { "ADC", 3, 3, 4, 1 },
    { "NOP", 6, 1, 2, 0 },
    { "RRA", 3, 0, 7, 0 },
    { "NOP", 2, 3, 4, 1 },
    { "ADC", 2, 3, 4, 1 },
    { "ROR", 2, 3, 7, 0 },
    { "RRA", 2, 0, 7, 0 },
    { "NOP", 5, 2, 2, 0 },
    { "STA", 7, 2, 6, 0 },
    { "NOP", 5, 0, 2, 0 },
    { "SAX", 7, 0, 6, 0 },
    { "STY", 11, 2, 3, 0 },
    { "STA", 11, 2, 3, 0 },
    { "STX", 11, 2, 3, 0 },
    { "SAX", 11, 0, 3, 0 },
    { "DEY", 6, 1, 2, 0 },
    { "NOP", 5, 0, 2, 0 },
    { "TXA", 6, 1, 2, 0 },
    { "XAA", 5, 0, 2, 0 },
    { "STY", 1, 3, 4, 0 },
    { "STA", 1, 3, 4, 0 },
    { "STX", 1, 3, 4, 0 },
    { "SAX", 1, 0, 4, 0 },
    { "BCC", 10, 2, 2, 1 },
    { "STA", 9, 2, 6, 0 },
    { "KIL", 6, 0, 2, 0 },
    { "AHX", 9, 0, 6, 0 },
    { "STY", 12, 2, 4, 0 },
    { "STA", 12, 2, 4, 0 },
    { "STX", 13, 2, 4, 0 },
    { "SAX", 13, 0, 4, 0 },
    { "TYA", 6, 1, 2, 0 },
    { "STA", 3, 3, 5, 0 },
    { "TXS", 6, 1, 2, 0 },
    { "TAS", 3, 0, 5, 0 },
    { "SHY", 2, 0, 5, 0 },
    { "STA", 2, 3, 5, 0 },
    { "SHX", 3, 0, 5, 0 },
    { "AHX", 3, 0, 5, 0 },
    { "LDY", 5, 2, 2, 0 },
    { "LDA", 7, 2, 6, 0 },
    { "LDX", 5, 2, 2, 0 },
    { "LAX", 7, 0, 6, 0 },
    { "LDY", 11, 2, 3, 0 },
    { "LDA", 11, 2, 3, 0 },
    { "LDX", 11, 2, 3, 0 },
    { "LAX", 11, 0, 3, 0 },
    { "TAY", 6, 1, 2, 0 },
    { "LDA", 5, 2, 2, 0 },
    { "TAX", 6, 1, 2, 0 },
    { "LAX", 5, 0, 2, 0 },
    { "LDY", 1, 3, 4, 0 },
    { "LDA", 1, 3, 4, 0 },
    { "LDX", 1, 3, 4, 0 },
    { "LAX", 1, 0, 4, 0 },
    { "BCS", 10, 2, 2, 1 },
    { "LDA", 9, 2, 5, 1 },
    { "KIL", 6, 0, 2, 0 },
    { "LAX", 9, 0, 5, 1 },
    { "LDY", 12, 2, 4, 0 },
    { "LDA", 12, 2, 4, 0 },
    { "LDX", 13, 2, 4, 0 },
    { "LAX", 13, 0, 4, 0 },
    { "CLV", 6, 1, 2, 0 },
    { "LDA", 3, 3, 4, 1 },
    { "TSX", 6, 1, 2, 0 },
    { "LAS", 3, 0, 4, 1 },
    { "LDY", 2, 3, 4, 1 },
    { "LDA", 2, 3, 4, 1 },
    { "LDX", 3, 3, 4, 1 },
    { "LAX", 3, 0, 4, 1 },
    { "CPY", 5, 2, 2, 0 },
    { "CMP", 7, 2, 6, 0 },
    { "NOP", 5, 0, 2, 0 },
    { "DCP", 7, 0, 8, 0 },
    { "CPY", 11, 2, 3, 0 },
    { "CMP", 11, 2, 3, 0 },
    { "DEC", 11, 2, 5, 0 },
    { "DCP", 11, 0, 5, 0 },
    { "INY", 6, 1, 2, 0 },
    { "CMP", 5, 2, 2, 0 },
    { "DEX", 6, 1, 2, 0 },
    { "AXS", 5, 0, 2, 0 },
    { "CPY", 1, 3, 4, 0 },
    { "CMP", 1, 3, 4, 0 },
    { "DEC", 1, 3, 6, 0 },
    { "DCP", 1, 0, 6, 0 },
    { "BNE", 10, 2, 2, 1 },
    { "CMP", 9, 2, 5, 1 },
    { "KIL", 6, 0, 2, 0 },
    { "DCP", 9, 0, 8, 0 },
    { "NOP", 12, 2, 4, 0 },
    { "CMP", 12, 2, 4, 0 },
    { "DEC", 12, 2, 6, 0 },
    { "DCP", 12, 0, 6, 0 },
    { "CLD", 6, 1, 2, 0 },
    { "CMP", 3, 3, 4, 1 },
    { "NOP", 6, 1, 2, 0 },
    { "DCP", 3, 0, 7, 0 },
    { "NOP", 2, 3, 4, 1 },
    { "CMP", 2, 3, 4, 1 },
    { "DEC", 2, 3, 7, 0 },
    { "DCP", 2, 0, 7, 0 },
    { "CPX", 5, 2, 2, 0 },
    { "SBC", 7, 2, 6, 0 },
    { "NOP", 5, 0, 2, 0 },
    { "ISC", 7, 0, 8, 0 },
    { "CPX", 11, 2, 3, 0 },
    { "SBC", 11, 2, 3, 0 },
    { "INC", 11, 2, 5, 0 },
    { "ISC", 11, 0, 5, 0 },
    { "INX", 6, 1, 2, 0 },
    { "SBC", 5, 2, 2, 0 },
    { "NOP", 6, 1, 2, 0 },
    { "SBC", 5, 0, 2, 0 },
    { "CPX", 1, 3, 4, 0 },
    { "SBC", 1, 3, 4, 0 },
    { "INC", 1, 3, 6, 0 },
    { "ISC", 1, 0, 6, 0 },
    { "BEQ", 10, 2, 2, 1 },
    { "SBC", 9, 2, 5, 1 },
    { "KIL", 6, 0, 2, 0 },
    { "ISC", 9, 0, 8, 0 },
    { "NOP", 12, 2, 4, 0 },
    { "SBC", 12, 2, 4, 0 },
    { "INC", 12, 2, 6, 0 },
    { "ISC", 12, 0, 6, 0 },
    { "SED", 6, 1, 2, 0 },
    { "SBC", 3, 3, 4, 1 },
    { "NOP", 6, 1, 2, 0 },
    { "ISC", 3, 0, 7, 0 },
    { "NOP", 2, 3, 4, 1 },
    { "SBC", 2, 3, 4, 1 },
    { "INC", 2, 3, 7, 0 },
    { "ISC", 2, 0, 7, 0 }
};

uint8_t * disassemble_step(uint8_t * offset) {

    struct instruction *op;

    op = &( instructionSet[*offset] );

    switch( op->mode ) {
        case MODE_IMPLIED:
        case MODE_ACCUMULATOR:
            printf("%s\n",op->name);
            break;
        case MODE_ABSOLUTE:
            printf("%s $%" PRIX8 "%" PRIX8 "\n", op->name, *(offset+2), *(offset+1) );
            break;
        case MODE_ABSOLUTE_X:
            printf("%s $%" PRIX8 "%" PRIX8 ",X\n", op->name, *(offset+2), *(offset+1) );
            break;
        case MODE_ABSOLUTE_Y:
            printf("%s $%" PRIX8 "%" PRIX8 ",Y\n", op->name, *(offset+2), *(offset+1) );
            break;
        case MODE_IMMEDIATE:
            printf("%s #$%" PRIX8 "\n", op->name, *(offset+1) );
        case MODE_INDEXED_INDIRECT:
            printf("%s ($%" PRIX8 ",X)\n", op->name, *(offset+1) );
        case MODE_INDIRECT:
            printf("%s ($%" PRIX8 "%" PRIX8 ")\n", op->name, *(offset+2), *(offset+1) );
        case MODE_INDIRECT_INDEXED:
            printf("%s ($%" PRIX8 "),Y\n", op->name, *(offset+1) );
        case MODE_RELATIVE:
        case MODE_ZERO_PAGE:
            printf("%s $%" PRIX8 "\n", op->name, (*offset+1) );
        case MODE_ZERO_PAGE_X:
            printf("%s $%" PRIX8 ",X\n", op->name, (*offset+1) );
        case MODE_ZERO_PAGE_Y:
            printf("%s $%" PRIX8 ",Y\n", op->name, (*offset+1) );

    }

    if (op->size==0) {
        printf("!!!\n");
    }

    return ( offset + op->size ) ;

}

void disassemble(uint8_t *rom) {

    uint8_t *offset;

    offset = rom;

    while( offset < rom + ROM_SIZE ) {
        offset = disassemble_step(offset);
    }


}