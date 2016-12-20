#include <inttypes.h>
#include <stdio.h>
#include "6502.h"

#define ON  1
#define OFF 0

#define NMI_VECTOR_LO 0xfffa
#define NMI_VECTOR_HI 0xfffb

// DATA LOCATIONS

#define LOC_NULL    0
#define LOC_REG_A   1
#define LOC_REG_X   2
#define LOC_REG_Y   3
#define LOC_REG_SP  4
#define LOC_REG_PC  5
#define LOC_REG_P   6
#define LOC_MEMORY  7
#define LOC_STACK   8

// ADDRESS MODE OPTIONS

#define ADDR_MODE_IMMEDIATE           ( 1 << 0 )
#define ADDR_MODE_INDIRECT            ( 1 << 1 )
#define ADDR_MODE_INDEX               ( 1 << 2 )
#define ADDR_MODE_INDEX_TIMING        ( 1 << 3 )
#define ADDR_MODE_INDEX_TIMING__PRE   0
#define ADDR_MODE_INDEX_TIMING__POST  ( 1 << 3 )
#define ADDR_MODE_INDEX_REG           ( 1 << 4 )
#define ADDR_MODE_INDEX_REG__X        0
#define ADDR_MODE_INDEX_REG__Y        ( 1 << 4 )
#define ADDR_MODE_RELATIVE            ( 1 << 5 )
#define ADDR_MODE_ACCUMULATOR         ( 1 << 6 )
#define ADDR_MODE                     ( 1 << 7 )

// ALU OPERATIONS

#define ALU_MODE_NOP             0
#define ALU_MODE_ADD             1
#define ALU_MODE_SUBTRACT        2
#define ALU_MODE_OR              3
#define ALU_MODE_AND             4
#define ALU_MODE_XOR             5
#define ALU_MODE_SHIFT_LEFT      6
#define ALU_MODE_SHIFT_RIGHT     7
#define ALU_MODE_ROTATE_LEFT     8
#define ALU_MODE_ROTATE_RIGHT    9
#define ALU_MODE_INCREMENT       10
#define ALU_MODE_DECREMENT       11
#define ALU_MODE_CMP             12

// BIT MASKS FOR STATUS REGISTER

#define STATUS_C  (1 << 0)  // CARRY
#define STATUS_Z  (1 << 1)  // ZERO
#define STATUS_I  (1 << 2)  // INTERRUPT DISABLE
#define STATUS_D  (1 << 3)  // DECIMAL MODE
#define STATUS_B  (1 << 4)  // BREAK (SOFTWARE INTERRUPT)
#define STATUS_V  (1 << 6)  // OVERFLOW
#define STATUS_N  (1 << 7)  // NEGATIVE

#define STATUS(CPU,MASK) ( !!( CPU->P & MASK ) )

#define READ(A) ( cpu->mm->read( cpu->mm,A ) )
#define READ16(A) ( ( ( uint16_t ) READ(A+1) << 8 ) | READ(A) )
#define WRITE(A,X) ( cpu->mm->write( cpu->mm,A,X) )

#define STACK_PUSH(X) ( WRITE( (cpu->SP)-- + 0x0100 , X ) )
#define STACK_PULL() ( READ( ++(cpu->SP) + 0x0100 ) )

enum AddrMode {
    ADDR_MODE__IMPLIED,
    ADDR_MODE__IMMEDIATE,
    ADDR_MODE__ACCUMULATOR,
    ADDR_MODE__ZERO_PAGE,
    ADDR_MODE__ABSOLUTE,
    ADDR_MODE__RELATIVE,
    ADDR_MODE__IDX_X,
    ADDR_MODE__IDX_Y,
    ADDR_MODE__ZERO_PAGE_IDX_X,
    ADDR_MODE__ZERO_PAGE_IDX_Y,
    ADDR_MODE__INDIRECT,
    ADDR_MODE__INDEX_INDIRECT,
    ADDR_MODE__INDIRECT_INDEX,
};

typedef enum AddrMode addr_mode_t;

enum Instruction {
    INSTRUCTION__NOP,
    INSTRUCTION__ADC,
    INSTRUCTION__AND,
    INSTRUCTION__ASL,
    INSTRUCTION__BCC,
    INSTRUCTION__BCS,
    INSTRUCTION__BEQ,
    INSTRUCTION__BIT,
    INSTRUCTION__BMI,
    INSTRUCTION__BNE,
    INSTRUCTION__BPL,
    INSTRUCTION__BRK,
    INSTRUCTION__BVC,
    INSTRUCTION__BVS,
    INSTRUCTION__CLC,
    INSTRUCTION__CLD,
    INSTRUCTION__CLI,
    INSTRUCTION__CLV,
    INSTRUCTION__CMP,
    INSTRUCTION__CPX,
    INSTRUCTION__CPY,
    INSTRUCTION__DEC,
    INSTRUCTION__DEX,
    INSTRUCTION__DEY,
    INSTRUCTION__EOR,
    INSTRUCTION__INC,
    INSTRUCTION__INX,
    INSTRUCTION__INY,
    INSTRUCTION__JMP,
    INSTRUCTION__JSR,
    INSTRUCTION__LDA,
    INSTRUCTION__LDX,
    INSTRUCTION__LDY,
    INSTRUCTION__LSR,
    INSTRUCTION__ORA,
    INSTRUCTION__PHA,
    INSTRUCTION__PHP,
    INSTRUCTION__PLA,
    INSTRUCTION__PLP,
    INSTRUCTION__ROL,
    INSTRUCTION__ROR,
    INSTRUCTION__RTI,
    INSTRUCTION__RTS,
    INSTRUCTION__SBC,
    INSTRUCTION__SEC,
    INSTRUCTION__SED,
    INSTRUCTION__SEI,
    INSTRUCTION__STA,
    INSTRUCTION__STX,
    INSTRUCTION__STY,
    INSTRUCTION__TAX,
    INSTRUCTION__TAY,
    INSTRUCTION__TSX,
    INSTRUCTION__TXA,
    INSTRUCTION__TXS,
    INSTRUCTION__TYA,
};

typedef enum Instruction instruction_t;

char instructionStringMap[56][4] = {
    "NOP",
    "ADC",
    "AND",
    "ASL",
    "BCC",
    "BCS",
    "BEQ",
    "BIT",
    "BMI",
    "BNE",
    "BPL",
    "BRK",
    "BVC",
    "BVS",
    "CLC",
    "CLD",
    "CLI",
    "CLV",
    "CMP",
    "CPX",
    "CPY",
    "DEC",
    "DEX",
    "DEY",
    "EOR",
    "INC",
    "INX",
    "INY",
    "JMP",
    "JSR",
    "LDA",
    "LDX",
    "LDY",
    "LSR",
    "ORA",
    "PHA",
    "PHP",
    "PLA",
    "PLP",
    "ROL",
    "ROR",
    "RTI",
    "RTS",
    "SBC",
    "SEC",
    "SED",
    "SEI",
    "STA",
    "STX",
    "STY",
    "TAX",
    "TAY",
    "TSX",
    "TXA",
    "TXS",
    "TYA",
};

// ADDRESS MODE DEFINITIONS

#define D_ADDR_MODE__IMPLIED \
        .addr_mode_type = ADDR_MODE__IMPLIED, \
        .size = 1,

#define D_ADDR_MODE__ACCUMULATOR \
        .addr_mode_type = ADDR_MODE__ACCUMULATOR, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_ACCUMULATOR ), \
        .size = 1,

#define D_ADDR_MODE__IMMEDIATE \
        .addr_mode_type = ADDR_MODE__IMMEDIATE, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_IMMEDIATE ), \
        .size = 2,

#define D_ADDR_MODE__ZERO_PAGE \
        .addr_mode_type = ADDR_MODE__ZERO_PAGE, \
        .addr_mode = ( ADDR_MODE ), \
        .size = 2,

#define D_ADDR_MODE__ABSOLUTE \
        .addr_mode_type = ADDR_MODE__ABSOLUTE, \
        .addr_mode = ( ADDR_MODE ), \
        .size = 3,

#define D_ADDR_MODE__RELATIVE \
        .addr_mode_type = ADDR_MODE__RELATIVE, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_RELATIVE ), \
        .size = 2,

#define D_ADDR_MODE__IDX_X \
        .addr_mode_type = ADDR_MODE__IDX_X, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_INDEX | ADDR_MODE_INDEX_REG__X ), \
        .size = 3,

#define D_ADDR_MODE__IDX_Y \
        .addr_mode_type = ADDR_MODE__IDX_Y, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_INDEX | ADDR_MODE_INDEX_REG__Y ), \
        .size = 3,

#define D_ADDR_MODE__ZERO_PAGE_IDX_X \
        .addr_mode_type = ADDR_MODE__ZERO_PAGE_IDX_X, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_INDEX | ADDR_MODE_INDEX_REG__X ), \
        .size = 2,

#define D_ADDR_MODE__ZERO_PAGE_IDX_Y \
        .addr_mode_type = ADDR_MODE__ZERO_PAGE_IDX_Y, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_INDEX | ADDR_MODE_INDEX_REG__X ), \
        .size = 2,

#define D_ADDR_MODE__INDIRECT \
        .addr_mode_type = ADDR_MODE__INDIRECT, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_INDIRECT ), \
        .size = 3, \

#define D_ADDR_MODE__INDEX_INDIRECT \
        .addr_mode_type = ADDR_MODE__INDEX_INDIRECT, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_INDEX | ADDR_MODE_INDEX_REG__X | ADDR_MODE_INDEX_TIMING__PRE | ADDR_MODE_INDIRECT ), \
        .size = 2,
#define D_ADDR_MODE__INDIRECT_INDEX \
        .addr_mode_type = ADDR_MODE__INDIRECT_INDEX, \
        .addr_mode = ( ADDR_MODE | ADDR_MODE_INDEX | ADDR_MODE_INDEX_REG__Y | ADDR_MODE_INDEX_TIMING__POST | ADDR_MODE_INDIRECT ), \
        .size = 2,

// INSTRUCTION DEFINITIONS

#define D_INSTRUCTION__ADC \
        .instruction_type = INSTRUCTION__ADC,\
        .src = LOC_MEMORY, \
        .aux = LOC_REG_A, \
        .dst = LOC_REG_A, \
        .alu_mode = ALU_MODE_ADD, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_V | STATUS_C ),

#define D_INSTRUCTION__AND \
        .instruction_type = INSTRUCTION__AND,\
        .src = LOC_MEMORY, \
        .aux = LOC_REG_A, \
        .dst = LOC_REG_A, \
        .alu_mode = ALU_MODE_AND, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__ASL \
        .instruction_type = INSTRUCTION__ASL,\
        .src = LOC_MEMORY, \
        .dst = LOC_MEMORY, \
        .alu_mode = ALU_MODE_SHIFT_LEFT, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_C ),

#define D_INSTRUCTION__BCC \
        .instruction_type = INSTRUCTION__BCC,\
        .branch_on = STATUS_C, \
        .branch_if = OFF,

#define D_INSTRUCTION__BCS \
        .instruction_type = INSTRUCTION__BCS,\
        .branch_on = STATUS_C, \
        .branch_if = ON,

#define D_INSTRUCTION__BEQ \
        .instruction_type = INSTRUCTION__BEQ,\
        .branch_on = STATUS_Z, \
        .branch_if = ON,

#define D_INSTRUCTION__BIT \
        .instruction_type = INSTRUCTION__BIT,\
        .src = LOC_MEMORY, \
        .aux = LOC_REG_A, \
        .dst = LOC_NULL, \
        .alu_mode = ALU_MODE_AND, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_V ),

#define D_INSTRUCTION__BMI \
        .instruction_type = INSTRUCTION__BMI,\
        .branch_on = STATUS_N, \
        .branch_if = ON,

#define D_INSTRUCTION__BNE \
        .instruction_type = INSTRUCTION__BNE,\
        .branch_on = STATUS_Z, \
        .branch_if = OFF,

#define D_INSTRUCTION__BPL \
        .instruction_type = INSTRUCTION__BPL,\
        .branch_on = STATUS_N, \
        .branch_if = OFF,

#define D_INSTRUCTION__BRK \
        .instruction_type = INSTRUCTION__BRK,\
        .status_mod = STATUS_B, \
        .status_val = ON,

#define D_INSTRUCTION__BVC \
        .instruction_type = INSTRUCTION__BVC,\
        .branch_on = STATUS_V, \
        .branch_if = OFF,

#define D_INSTRUCTION__BVS \
        .instruction_type = INSTRUCTION__BVS,\
        .branch_on = STATUS_V, \
        .branch_if = OFF,

#define D_INSTRUCTION__CLC \
        .instruction_type = INSTRUCTION__CLC,\
        .status_mod = STATUS_C, \
        .status_val = OFF,

#define D_INSTRUCTION__CLD \
        .instruction_type = INSTRUCTION__CLD,\
        .status_mod = STATUS_D, \
        .status_val = OFF,

#define D_INSTRUCTION__CLI \
        .instruction_type = INSTRUCTION__CLI,\
        .status_mod = STATUS_I, \
        .status_val = OFF,

#define D_INSTRUCTION__CLV \
        .instruction_type = INSTRUCTION__CLV,\
        .status_mod = STATUS_V, \
        .status_val = OFF,

#define D_INSTRUCTION__CMP \
        .instruction_type = INSTRUCTION__CMP,\
        .src = LOC_MEMORY, \
        .aux = LOC_REG_A, \
        .dst = LOC_NULL, \
        .alu_mode = ALU_MODE_CMP, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_C ),

#define D_INSTRUCTION__CPX \
        .instruction_type = INSTRUCTION__CPX,\
        .src = LOC_MEMORY, \
        .aux = LOC_REG_X, \
        .dst = LOC_NULL, \
        .alu_mode = ALU_MODE_CMP, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_C ),

#define D_INSTRUCTION__CPY \
        .instruction_type = INSTRUCTION__CPY,\
        .src = LOC_MEMORY, \
        .aux = LOC_REG_Y, \
        .dst = LOC_NULL, \
        .alu_mode = ALU_MODE_CMP, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_C ),

#define D_INSTRUCTION__DEC \
        .instruction_type = INSTRUCTION__DEC,\
        .src = LOC_MEMORY, \
        .dst = LOC_MEMORY, \
        .alu_mode = ALU_MODE_DECREMENT, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__DEX \
        .instruction_type = INSTRUCTION__DEX,\
        .src = LOC_REG_X, \
        .dst = LOC_REG_X, \
        .alu_mode = ALU_MODE_DECREMENT, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__DEY \
        .instruction_type = INSTRUCTION__DEY,\
        .src = LOC_REG_Y, \
        .dst = LOC_REG_Y, \
        .alu_mode = ALU_MODE_DECREMENT, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__EOR \
        .instruction_type = INSTRUCTION__EOR,\
        .src = LOC_MEMORY, \
        .aux = LOC_REG_A, \
        .dst = LOC_REG_A, \
        .alu_mode = ALU_MODE_XOR, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__INC \
        .instruction_type = INSTRUCTION__INC,\
        .src = LOC_MEMORY, \
        .dst = LOC_MEMORY, \
        .alu_mode = ALU_MODE_INCREMENT, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__INX \
        .instruction_type = INSTRUCTION__INX,\
        .src = LOC_REG_X, \
        .dst = LOC_REG_X, \
        .alu_mode = ALU_MODE_INCREMENT, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__INY \
        .instruction_type = INSTRUCTION__INY,\
        .src = LOC_REG_Y, \
        .dst = LOC_REG_Y, \
        .alu_mode = ALU_MODE_INCREMENT, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__JMP \
        .instruction_type = INSTRUCTION__JMP,

#define D_INSTRUCTION__JSR \
        .instruction_type = INSTRUCTION__JSR,

#define D_INSTRUCTION__LDA \
        .instruction_type = INSTRUCTION__LDA,\
        .src = LOC_MEMORY, \
        .dst = LOC_REG_A, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__LDX \
        .instruction_type = INSTRUCTION__LDX,\
        .src = LOC_MEMORY, \
        .dst = LOC_REG_X, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__LDY \
        .instruction_type = INSTRUCTION__LDY,\
        .src = LOC_MEMORY, \
        .dst = LOC_REG_Y, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__LSR \
        .instruction_type = INSTRUCTION__LSR,\
        .src = LOC_MEMORY, \
        .dst = LOC_MEMORY, \
        .alu_mode = ALU_MODE_SHIFT_RIGHT, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_C ),

#define D_INSTRUCTION__NOP \
        .instruction_type = INSTRUCTION__NOP,\

#define D_INSTRUCTION__ORA \
        .instruction_type = INSTRUCTION__ORA,\
        .src = LOC_MEMORY, \
        .aux = LOC_REG_A, \
        .dst = LOC_REG_A, \
        .alu_mode = ALU_MODE_OR, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__PHA \
        .instruction_type = INSTRUCTION__PHA,\
        .src = LOC_REG_A, \
        .dst = LOC_STACK, \

#define D_INSTRUCTION__PHP \
        .instruction_type = INSTRUCTION__PHP,\
        .src = LOC_REG_P, \
        .dst = LOC_STACK, \

#define D_INSTRUCTION__PLA \
        .instruction_type = INSTRUCTION__PLA,\
        .src = LOC_STACK, \
        .dst = LOC_REG_A, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__PLP \
        .instruction_type = INSTRUCTION__PLP,\
        .src = LOC_STACK, \
        .dst = LOC_REG_P, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__ROL \
        .instruction_type = INSTRUCTION__ROL,\
        .src = LOC_MEMORY, \
        .dst = LOC_MEMORY, \
        .alu_mode = ALU_MODE_ROTATE_LEFT, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_C ),

#define D_INSTRUCTION__ROR \
        .instruction_type = INSTRUCTION__ROR,\
        .src = LOC_MEMORY, \
        .dst = LOC_MEMORY, \
        .alu_mode = ALU_MODE_ROTATE_RIGHT, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_C ),

#define D_INSTRUCTION__RTI \
        .instruction_type = INSTRUCTION__RTI,\

#define D_INSTRUCTION__RTS \
        .instruction_type = INSTRUCTION__RTS,\

#define D_INSTRUCTION__SBC \
        .instruction_type = INSTRUCTION__SBC,\
        .src = LOC_MEMORY, \
        .aux = LOC_REG_A, \
        .dst = LOC_REG_A, \
        .alu_mode = ALU_MODE_SUBTRACT, \
        .status_update = ( STATUS_Z | STATUS_N | STATUS_C | STATUS_V ),

#define D_INSTRUCTION__SEC \
        .instruction_type = INSTRUCTION__SEC,\
        .status_mod = STATUS_C, \
        .status_val = ON,

#define D_INSTRUCTION__SED \
        .instruction_type = INSTRUCTION__SED,\
        .status_mod = STATUS_D, \
        .status_val = ON,

#define D_INSTRUCTION__SEI \
        .instruction_type = INSTRUCTION__SEI,\
        .status_mod = STATUS_I, \
        .status_val = ON,

#define D_INSTRUCTION__STA \
        .instruction_type = INSTRUCTION__STA,\
        .src = LOC_REG_A, \
        .dst = LOC_MEMORY,

#define D_INSTRUCTION__STX \
        .instruction_type = INSTRUCTION__STX,\
        .src = LOC_REG_X, \
        .dst = LOC_MEMORY, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__STY \
        .instruction_type = INSTRUCTION__STY,\
        .src = LOC_REG_Y, \
        .dst = LOC_MEMORY, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__TAX \
        .instruction_type = INSTRUCTION__TAX,\
        .src = LOC_REG_A, \
        .dst = LOC_REG_X, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__TAY \
        .instruction_type = INSTRUCTION__TAY,\
        .src = LOC_REG_A, \
        .dst = LOC_REG_Y, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__TSX \
        .instruction_type = INSTRUCTION__TSX,\
        .src = LOC_REG_SP, \
        .dst = LOC_REG_X, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__TXA \
        .instruction_type = INSTRUCTION__TXA,\
        .src = LOC_REG_X, \
        .dst = LOC_REG_A, \
        .status_update = ( STATUS_Z | STATUS_N ),

#define D_INSTRUCTION__TXS \
        .instruction_type = INSTRUCTION__TXS,\
        .src = LOC_REG_X, \
        .dst = LOC_REG_SP,

#define D_INSTRUCTION__TYA \
        .instruction_type = INSTRUCTION__TYA,\
        .src = LOC_REG_Y, \
        .dst = LOC_REG_A, \
        .status_update = ( STATUS_Z | STATUS_N ),

struct operation {

    addr_mode_t addr_mode_type;
    instruction_t instruction_type;

    unsigned int size;
    unsigned int addr_mode;

    unsigned int src;
    unsigned int aux;
    unsigned int dst;
    unsigned int alu_mode;

    uint8_t branch_on;
    uint8_t branch_if;

    uint8_t status_update;

    uint8_t status_mod;
    uint8_t status_val;

};

struct operation instructionSet[256] = {

    [0x69] = { D_INSTRUCTION__ADC  D_ADDR_MODE__IMMEDIATE },
    [0x65] = { D_INSTRUCTION__ADC  D_ADDR_MODE__ZERO_PAGE },
    [0x75] = { D_INSTRUCTION__ADC  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x6d] = { D_INSTRUCTION__ADC  D_ADDR_MODE__ABSOLUTE },
    [0x7d] = { D_INSTRUCTION__ADC  D_ADDR_MODE__IDX_X },
    [0x79] = { D_INSTRUCTION__ADC  D_ADDR_MODE__IDX_Y },
    [0x61] = { D_INSTRUCTION__ADC  D_ADDR_MODE__INDEX_INDIRECT },
    [0x71] = { D_INSTRUCTION__ADC  D_ADDR_MODE__INDIRECT_INDEX },

    [0x29] = { D_INSTRUCTION__AND  D_ADDR_MODE__IMMEDIATE },
    [0x25] = { D_INSTRUCTION__AND  D_ADDR_MODE__ZERO_PAGE },
    [0x35] = { D_INSTRUCTION__AND  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x2d] = { D_INSTRUCTION__AND  D_ADDR_MODE__ABSOLUTE },
    [0x3d] = { D_INSTRUCTION__AND  D_ADDR_MODE__IDX_X },
    [0x39] = { D_INSTRUCTION__AND  D_ADDR_MODE__IDX_Y },
    [0x21] = { D_INSTRUCTION__AND  D_ADDR_MODE__INDEX_INDIRECT },
    [0x31] = { D_INSTRUCTION__AND  D_ADDR_MODE__INDIRECT_INDEX },

    [0x0a] = { D_INSTRUCTION__ASL  D_ADDR_MODE__ACCUMULATOR },
    [0x06] = { D_INSTRUCTION__ASL  D_ADDR_MODE__ZERO_PAGE },
    [0x16] = { D_INSTRUCTION__ASL  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x0e] = { D_INSTRUCTION__ASL  D_ADDR_MODE__ABSOLUTE },
    [0x1e] = { D_INSTRUCTION__ASL  D_ADDR_MODE__IDX_X },

    [0x90] = { D_INSTRUCTION__BCC  D_ADDR_MODE__RELATIVE },

    [0xb0] = { D_INSTRUCTION__BCS  D_ADDR_MODE__RELATIVE },

    [0xf0] = { D_INSTRUCTION__BEQ  D_ADDR_MODE__RELATIVE },

    [0x24] = { D_INSTRUCTION__BIT  D_ADDR_MODE__ZERO_PAGE },
    [0x2c] = { D_INSTRUCTION__BIT  D_ADDR_MODE__ABSOLUTE },

    [0x30] = { D_INSTRUCTION__BMI  D_ADDR_MODE__RELATIVE },

    [0xd0] = { D_INSTRUCTION__BNE  D_ADDR_MODE__RELATIVE },

    [0x10] = { D_INSTRUCTION__BPL  D_ADDR_MODE__RELATIVE },

    [0x00] = { D_INSTRUCTION__BRK  D_ADDR_MODE__IMPLIED },

    [0x50] = { D_INSTRUCTION__BVC  D_ADDR_MODE__RELATIVE },

    [0x70] = { D_INSTRUCTION__BVS  D_ADDR_MODE__RELATIVE },

    [0x18] = { D_INSTRUCTION__CLC  D_ADDR_MODE__IMPLIED },

    [0xd8] = { D_INSTRUCTION__CLD  D_ADDR_MODE__IMPLIED },

    [0x58] = { D_INSTRUCTION__CLI  D_ADDR_MODE__IMPLIED },

    [0xb8] = { D_INSTRUCTION__CLV  D_ADDR_MODE__IMPLIED },

    [0xc9] = { D_INSTRUCTION__CMP  D_ADDR_MODE__IMMEDIATE },
    [0xc5] = { D_INSTRUCTION__CMP  D_ADDR_MODE__ZERO_PAGE },
    [0xd5] = { D_INSTRUCTION__CMP  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0xcd] = { D_INSTRUCTION__CMP  D_ADDR_MODE__ABSOLUTE },
    [0xdd] = { D_INSTRUCTION__CMP  D_ADDR_MODE__IDX_X },
    [0xd9] = { D_INSTRUCTION__CMP  D_ADDR_MODE__IDX_Y },
    [0xc1] = { D_INSTRUCTION__CMP  D_ADDR_MODE__INDEX_INDIRECT },
    [0xd1] = { D_INSTRUCTION__CMP  D_ADDR_MODE__INDIRECT_INDEX },

    [0xe0] = { D_INSTRUCTION__CPX  D_ADDR_MODE__IMMEDIATE },
    [0xe4] = { D_INSTRUCTION__CPX  D_ADDR_MODE__ZERO_PAGE },   
    [0xec] = { D_INSTRUCTION__CPX  D_ADDR_MODE__ABSOLUTE },

    [0xc0] = { D_INSTRUCTION__CPY  D_ADDR_MODE__IMMEDIATE },
    [0xc4] = { D_INSTRUCTION__CPY  D_ADDR_MODE__ZERO_PAGE },   
    [0xcc] = { D_INSTRUCTION__CPY  D_ADDR_MODE__ABSOLUTE },

    [0xc6] = { D_INSTRUCTION__DEC  D_ADDR_MODE__ZERO_PAGE },
    [0xd6] = { D_INSTRUCTION__DEC  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0xce] = { D_INSTRUCTION__DEC  D_ADDR_MODE__ABSOLUTE },
    [0xde] = { D_INSTRUCTION__DEC  D_ADDR_MODE__IDX_X },

    [0xca] = { D_INSTRUCTION__DEX  D_ADDR_MODE__IMPLIED },

    [0x88] = { D_INSTRUCTION__DEY  D_ADDR_MODE__IMPLIED },

    [0x49] = { D_INSTRUCTION__EOR  D_ADDR_MODE__IMMEDIATE },
    [0x45] = { D_INSTRUCTION__EOR  D_ADDR_MODE__ZERO_PAGE },
    [0x55] = { D_INSTRUCTION__EOR  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x4d] = { D_INSTRUCTION__EOR  D_ADDR_MODE__ABSOLUTE },
    [0x5d] = { D_INSTRUCTION__EOR  D_ADDR_MODE__IDX_X },
    [0x59] = { D_INSTRUCTION__EOR  D_ADDR_MODE__IDX_Y },
    [0x41] = { D_INSTRUCTION__EOR  D_ADDR_MODE__INDEX_INDIRECT },
    [0x51] = { D_INSTRUCTION__EOR  D_ADDR_MODE__INDIRECT_INDEX },
    
    [0xe6] = { D_INSTRUCTION__INC  D_ADDR_MODE__ZERO_PAGE },
    [0xf6] = { D_INSTRUCTION__INC  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0xee] = { D_INSTRUCTION__INC  D_ADDR_MODE__ABSOLUTE },
    [0xfe] = { D_INSTRUCTION__INC  D_ADDR_MODE__IDX_X },

    [0xe8] = { D_INSTRUCTION__INX  D_ADDR_MODE__IMPLIED },
    [0xc8] = { D_INSTRUCTION__INY  D_ADDR_MODE__IMPLIED },

    [0x4c] = { D_INSTRUCTION__JMP  D_ADDR_MODE__ABSOLUTE },
    [0x6c] = { D_INSTRUCTION__JMP  D_ADDR_MODE__INDIRECT },

    [0x20] = { D_INSTRUCTION__JSR  D_ADDR_MODE__ABSOLUTE },

    [0xa9] = { D_INSTRUCTION__LDA  D_ADDR_MODE__IMMEDIATE },
    [0xa5] = { D_INSTRUCTION__LDA  D_ADDR_MODE__ZERO_PAGE },
    [0xb5] = { D_INSTRUCTION__LDA  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0xad] = { D_INSTRUCTION__LDA  D_ADDR_MODE__ABSOLUTE },
    [0xbd] = { D_INSTRUCTION__LDA  D_ADDR_MODE__IDX_X },
    [0xb9] = { D_INSTRUCTION__LDA  D_ADDR_MODE__IDX_Y },
    [0xa1] = { D_INSTRUCTION__LDA  D_ADDR_MODE__INDEX_INDIRECT },
    [0xb1] = { D_INSTRUCTION__LDA  D_ADDR_MODE__INDIRECT_INDEX },

    [0xa2] = { D_INSTRUCTION__LDX  D_ADDR_MODE__IMMEDIATE },
    [0xa6] = { D_INSTRUCTION__LDX  D_ADDR_MODE__ZERO_PAGE },
    [0xb6] = { D_INSTRUCTION__LDX  D_ADDR_MODE__ZERO_PAGE_IDX_Y },
    [0xae] = { D_INSTRUCTION__LDX  D_ADDR_MODE__ABSOLUTE },
    [0xbe] = { D_INSTRUCTION__LDX  D_ADDR_MODE__IDX_Y },

    [0xa0] = { D_INSTRUCTION__LDY  D_ADDR_MODE__IMMEDIATE },
    [0xa4] = { D_INSTRUCTION__LDY  D_ADDR_MODE__ZERO_PAGE },
    [0xb4] = { D_INSTRUCTION__LDY  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0xac] = { D_INSTRUCTION__LDY  D_ADDR_MODE__ABSOLUTE },
    [0xbc] = { D_INSTRUCTION__LDY  D_ADDR_MODE__IDX_X },
    
    [0x4a] = { D_INSTRUCTION__LSR  D_ADDR_MODE__ACCUMULATOR },
    [0x46] = { D_INSTRUCTION__LSR  D_ADDR_MODE__ZERO_PAGE },
    [0x56] = { D_INSTRUCTION__LSR  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x4e] = { D_INSTRUCTION__LSR  D_ADDR_MODE__ABSOLUTE },
    [0x5e] = { D_INSTRUCTION__LSR  D_ADDR_MODE__IDX_X },

    [0xea] = { D_INSTRUCTION__NOP  D_ADDR_MODE__IMPLIED},

    [0x09] = { D_INSTRUCTION__ORA  D_ADDR_MODE__IMMEDIATE },
    [0x05] = { D_INSTRUCTION__ORA  D_ADDR_MODE__ZERO_PAGE },
    [0x15] = { D_INSTRUCTION__ORA  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x0d] = { D_INSTRUCTION__ORA  D_ADDR_MODE__ABSOLUTE },
    [0x1d] = { D_INSTRUCTION__ORA  D_ADDR_MODE__IDX_X },
    [0x19] = { D_INSTRUCTION__ORA  D_ADDR_MODE__IDX_Y },
    [0x01] = { D_INSTRUCTION__ORA  D_ADDR_MODE__INDEX_INDIRECT },
    [0x11] = { D_INSTRUCTION__ORA  D_ADDR_MODE__INDIRECT_INDEX },

    [0x48] = { D_INSTRUCTION__PHA  D_ADDR_MODE__IMPLIED },

    [0x08] = { D_INSTRUCTION__PHP  D_ADDR_MODE__IMPLIED },

    [0x68] = { D_INSTRUCTION__PLA  D_ADDR_MODE__IMPLIED },
    
    [0x28] = { D_INSTRUCTION__PLP  D_ADDR_MODE__IMPLIED },

    [0x2a] = { D_INSTRUCTION__ROL  D_ADDR_MODE__ACCUMULATOR },
    [0x26] = { D_INSTRUCTION__ROL  D_ADDR_MODE__ZERO_PAGE },
    [0x36] = { D_INSTRUCTION__ROL  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x2e] = { D_INSTRUCTION__ROL  D_ADDR_MODE__ABSOLUTE },
    [0x3e] = { D_INSTRUCTION__ROL  D_ADDR_MODE__IDX_X },

    [0x6a] = { D_INSTRUCTION__ROR  D_ADDR_MODE__ACCUMULATOR },
    [0x66] = { D_INSTRUCTION__ROR  D_ADDR_MODE__ZERO_PAGE },
    [0x76] = { D_INSTRUCTION__ROR  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x6e] = { D_INSTRUCTION__ROR  D_ADDR_MODE__ABSOLUTE },
    [0x7e] = { D_INSTRUCTION__ROR  D_ADDR_MODE__IDX_X },

    [0x40] = { D_INSTRUCTION__RTI  D_ADDR_MODE__IMPLIED },

    [0x60] = { D_INSTRUCTION__RTS  D_ADDR_MODE__IMPLIED},

    [0xe9] = { D_INSTRUCTION__SBC  D_ADDR_MODE__IMMEDIATE },
    [0xe5] = { D_INSTRUCTION__SBC  D_ADDR_MODE__ZERO_PAGE },
    [0xf5] = { D_INSTRUCTION__SBC  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0xed] = { D_INSTRUCTION__SBC  D_ADDR_MODE__ABSOLUTE },
    [0xfd] = { D_INSTRUCTION__SBC  D_ADDR_MODE__IDX_X },
    [0xf9] = { D_INSTRUCTION__SBC  D_ADDR_MODE__IDX_Y },
    [0xe1] = { D_INSTRUCTION__SBC  D_ADDR_MODE__INDEX_INDIRECT },
    [0xf1] = { D_INSTRUCTION__SBC  D_ADDR_MODE__INDIRECT_INDEX },

    [0x38] = { D_INSTRUCTION__SEC  D_ADDR_MODE__IMPLIED },
    
    [0xf8] = { D_INSTRUCTION__SED  D_ADDR_MODE__IMPLIED },

    [0x78] = { D_INSTRUCTION__SEI  D_ADDR_MODE__IMPLIED },

    [0x85] = { D_INSTRUCTION__STA  D_ADDR_MODE__ZERO_PAGE },
    [0x95] = { D_INSTRUCTION__STA  D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x8d] = { D_INSTRUCTION__STA  D_ADDR_MODE__ABSOLUTE },
    [0x9d] = { D_INSTRUCTION__STA  D_ADDR_MODE__IDX_X },
    [0x99] = { D_INSTRUCTION__STA  D_ADDR_MODE__IDX_Y },
    [0x81] = { D_INSTRUCTION__STA  D_ADDR_MODE__INDEX_INDIRECT },
    [0x91] = { D_INSTRUCTION__STA  D_ADDR_MODE__INDIRECT_INDEX },

    [0x86] = { D_INSTRUCTION__STX D_ADDR_MODE__ZERO_PAGE },
    [0x96] = { D_INSTRUCTION__STX D_ADDR_MODE__ZERO_PAGE_IDX_Y },
    [0x8e] = { D_INSTRUCTION__STX D_ADDR_MODE__ABSOLUTE },

    [0x84] = { D_INSTRUCTION__STY D_ADDR_MODE__ZERO_PAGE },
    [0x94] = { D_INSTRUCTION__STY D_ADDR_MODE__ZERO_PAGE_IDX_X },
    [0x8c] = { D_INSTRUCTION__STY D_ADDR_MODE__ABSOLUTE },

    [0xaa] = { D_INSTRUCTION__TAX D_ADDR_MODE__IMPLIED },

    [0xa8] = { D_INSTRUCTION__TAY D_ADDR_MODE__IMPLIED },

    [0xba] = { D_INSTRUCTION__TSX D_ADDR_MODE__IMPLIED },

    [0x8a] = { D_INSTRUCTION__TXA D_ADDR_MODE__IMPLIED },

    [0x9a] = { D_INSTRUCTION__TXS D_ADDR_MODE__IMPLIED },

    [0x98] = { D_INSTRUCTION__TYA D_ADDR_MODE__IMPLIED },

};

void cpu6502PrintDebugInfo( struct cpu6502 *cpu ) {
    int i;

    printf("<%d>\n",cpu->clk);
    printf("0x%" PRIx8 " 0x%" PRIx8 " 0x%" PRIx8 "\n", cpu->opcode,cpu->o1,cpu->o2);
    printf("A: $%02" PRIx8 " ",cpu->A);
    printf("X: $%02" PRIx8 " ",cpu->X);
    printf("Y: $%02" PRIx8 " ",cpu->Y);
    printf("SP: 0x%" PRIx8 " ",cpu->SP);
    printf("P: $%02" PRIx8 "\n",cpu->P);
    printf("|N|V|?|B|D|I|Z|C|\n");
    for(i=7;i>=0;i--) printf("|%u",( (cpu->P)>>i & 0x01 ) );
    printf("|\n");
    printf("PC: 0x%" PRIx16 "\n",cpu->PC);
    printf("------------------------------------------------------\n");
    return;
}

void cpu6502PrintInstruction( struct cpu6502 *cpu ) {

    struct operation op = instructionSet[cpu->opcode];
    printf("{");
    printf("%s ", instructionStringMap[ op.instruction_type ] );

    switch( op.addr_mode_type ) {
        case ADDR_MODE__IMPLIED:
            break;
        case ADDR_MODE__IMMEDIATE:
            printf("#%02" PRIX8, cpu->o1);
            break;
        case ADDR_MODE__ACCUMULATOR:
            printf("A");
            break;
        case ADDR_MODE__ZERO_PAGE:
            printf("$%02" PRIX8, cpu->o1);
            break;
        case ADDR_MODE__ABSOLUTE:
            printf("$%02" PRIX8 "%02" PRIX8, cpu->o2, cpu->o1);
            break;
        case ADDR_MODE__RELATIVE:
            printf("%+" PRId8, *( (int8_t *) &(cpu->o1) ) );
            break;
        case ADDR_MODE__IDX_X:
            printf("$%02" PRIX8 "%02" PRIX8 ",X", cpu->o2, cpu->o1);
            break;
        case ADDR_MODE__IDX_Y:
            printf("$%02" PRIX8 "%02" PRIX8 ",Y", cpu->o2, cpu->o1);
            break;
        case ADDR_MODE__ZERO_PAGE_IDX_X:
            printf("$%02" PRIX8 ",X", cpu->o1);
            break;
        case ADDR_MODE__ZERO_PAGE_IDX_Y:
            printf("$%02" PRIX8 ",Y", cpu->o1);
            break;
        case ADDR_MODE__INDIRECT:
            printf("($%02" PRIX8 "%02" PRIX8 ")", cpu->o2, cpu->o1);
            break;
        case ADDR_MODE__INDEX_INDIRECT:
            printf("($%02" PRIX8 ",X)", cpu->o1);
            break;
        case ADDR_MODE__INDIRECT_INDEX:
            printf("($%02" PRIX8 "),Y", cpu->o1);
            break;
    }

    printf("}\n");

}

void cpu6502Init( struct cpu6502 * cpu ) {
    cpu->PC = 0x8000;
    cpu->SP = 0xfd;
    cpu->A = 0x00;
    cpu->X = 0x00;
    cpu->Y = 0x00;
    cpu->P = 0x34;
}

void cpu6502Step( struct cpu6502 *cpu, cpu6502Signal sig ) {

    struct operation op;
    uint8_t idx;
    uint16_t addr;
    uint8_t srcv;
    uint8_t auxv;
    uint8_t dstv;
    uint8_t tmpc;
    uint8_t tmpv;
    uint16_t talu;
    uint16_t temppc;
    
    cpu->clk++;
    
    // INSTRUCTION FETCH AND DECODE

    cpu->opcode = READ( cpu->PC );

    op = instructionSet[cpu->opcode];

    if ( op.size >= 2 ) {
        cpu->o1 = READ( cpu->PC + 1 );
    }

    if ( op.size >= 3 ) {
        cpu->o2 = READ( cpu->PC + 2 ) ;
    }


    // ADDRESS CALCULATION


    if ( op.addr_mode & ADDR_MODE ) {

        idx = 0;

        if ( op.addr_mode & ADDR_MODE_IMMEDIATE ) {
            addr = cpu->PC + 1;
        } else if ( op.addr_mode & ADDR_MODE_RELATIVE ) {
            addr = cpu->PC + op.size + *( (int8_t *) &(cpu->o1) );
        } else {


            if ( op.addr_mode & ADDR_MODE_INDEX  ) {
                if ( ( op.addr_mode & ADDR_MODE_INDEX_REG ) == ADDR_MODE_INDEX_REG__X ) {
                    idx = cpu->X;
                } else {
                    idx = cpu->Y;
                }
            }
                
            if ( op.size > 2 ) {
                addr = ( ( (uint16_t) ( cpu->o2 ) ) << 8 ) + cpu->o1;

            } else {
                addr = ( cpu->o1 );
            }

            if ( ( op.addr_mode & ADDR_MODE_INDEX ) && (( op.addr_mode & ADDR_MODE_INDEX_TIMING ) == ADDR_MODE_INDEX_TIMING__PRE ) ) {
                addr = ( addr + idx );
                if ( op.size <= 2 ) {
                    addr %= 0x100;
                }
            }

            if ( op.addr_mode & ADDR_MODE_INDIRECT ) {
                addr = READ16( addr );
                if ( ( op.addr_mode & ADDR_MODE_INDEX ) && ( ( op.addr_mode & ADDR_MODE_INDEX_TIMING ) == ADDR_MODE_INDEX_TIMING__POST ) ) {
                    addr += idx;
                }

            }
        }
    }


    // READ SOURCE

    switch( op.src ) {
        case LOC_MEMORY:
            if ( op.addr_mode & ADDR_MODE_ACCUMULATOR ) {
                srcv = cpu->A;
            }
            else {
                srcv = READ(addr);
            }
            break;
        case LOC_STACK:
            srcv = STACK_PULL();
            break;
        case LOC_REG_A:
            srcv = cpu->A;
            break;
        case LOC_REG_X:
            srcv = cpu->X;
            break;
        case LOC_REG_Y:
            srcv = cpu->Y;
            break;
        case LOC_REG_SP:
            srcv = cpu->SP;
            break;
        case LOC_REG_P:
            srcv = cpu->P;
            break;

    }

    switch ( op.aux ) {
        case LOC_REG_A:
            auxv = cpu->A;
            break;
        case LOC_REG_X:
            auxv = cpu->X;
            break;
        case LOC_REG_Y:
            auxv = cpu->Y;
            break;
    }

    // ALU EXECUTION

    switch ( op.alu_mode ) {

        case ALU_MODE_ADD:
            talu = cpu->A + srcv + STATUS(cpu,STATUS_C);
            tmpc = ( talu > 0xff );
            tmpv = !((auxv ^ srcv) & 0x80) && (auxv ^ talu) & 0x80;
            break;
        case ALU_MODE_SUBTRACT:
            talu = cpu->A - srcv - !STATUS(cpu,STATUS_C);
            tmpc = ( talu <= 0xff );
            tmpv = ((auxv ^ talu) & 0x80) && (auxv ^ srcv) & 0x80;
            break;
        case ALU_MODE_CMP:
            dstv = auxv - srcv;
            tmpc = auxv >= srcv;
            break;
        case ALU_MODE_XOR:
            dstv = cpu->A ^ srcv;
            break;
        case ALU_MODE_OR:
            dstv = cpu->A | srcv;
            break;
        case ALU_MODE_AND:
            dstv = cpu->A & srcv;
            if (op.instruction_type == INSTRUCTION__BIT) {
                tmpv = !!( dstv & 0x40 );
            }
            break;
        case ALU_MODE_SHIFT_LEFT:
        case ALU_MODE_ROTATE_LEFT:
            tmpc = !!( srcv & 0x80 );
            dstv = srcv << 1;
            if ( op.alu_mode == ALU_MODE_ROTATE_LEFT ) {
                dstv = dstv | !!( cpu->P & STATUS_C );
            }
            break;
        case ALU_MODE_SHIFT_RIGHT:
        case ALU_MODE_ROTATE_RIGHT:
            printf("++\n");

            tmpc = !!( srcv & 0x01 );
            dstv = ( 0x7f & ( srcv >> 1 ) );
            if ( op.alu_mode == ALU_MODE_ROTATE_RIGHT ) {
                dstv = dstv | !!( cpu->P & STATUS_C )<<7;
            }
            break;
        case ALU_MODE_INCREMENT:
            dstv = srcv + 1;
            break;
        case ALU_MODE_DECREMENT:
            dstv = srcv - 1;
            break;
        default:
            dstv = srcv;

    }

    // EVALUATE STATUS

    if ( op.status_update & STATUS_Z ) {
        cpu->P = ( cpu->P & ~(STATUS_Z ) ) | ( !(dstv) * STATUS_Z );
    }
    if ( op.status_update & STATUS_N ) {
        cpu->P = ( cpu->P & ~(STATUS_N ) ) | ( !!( dstv & 0x80 ) * STATUS_N );
    }
    if ( op.status_update & STATUS_V ) {
        cpu->P = ( cpu->P & ~(STATUS_V) ) | ( tmpv * STATUS_V );
    }
    if ( op.status_update & STATUS_C) {
        cpu->P = ( cpu->P & ~(STATUS_C) ) | ( tmpc * STATUS_C );
    }


    // WRITE DST

    switch( op.dst ) {
        case LOC_MEMORY:
            if ( op.addr_mode & ADDR_MODE_ACCUMULATOR ) {
                cpu->A = dstv;
            }
            else {
                WRITE(addr,dstv);
            }
            break;
        case LOC_STACK:
            STACK_PUSH(dstv);
        case LOC_REG_A:
            cpu->A = dstv;
            break;
        case LOC_REG_X:
            cpu->X = dstv;
            break;
        case LOC_REG_Y:
            cpu->Y = dstv;
            break;
        case LOC_REG_SP:
            cpu->SP = dstv;
            break;
        case LOC_REG_P:
            cpu->P = dstv;
            break;
    }

    // STATUS CTRL

    if ( op.status_mod ) {
        cpu->P = ( cpu->P & ~(op.status_mod) ) | ( op.status_val * op.status_mod );
    }

    // BRANCH (needs cleanup)

    if ( (op.instruction_type == INSTRUCTION__JMP) || ( op.branch_on && ( !( !!( op.branch_on & cpu->P ) ^ !!(op.branch_if) ) ) ) ) {
        cpu->PC = addr;
    } else if ( op.instruction_type == INSTRUCTION__JSR ) {
        temppc = cpu->PC + 2;
        STACK_PUSH( *( ( (uint8_t *) &temppc ) + 1 ) );
        STACK_PUSH( *( (uint8_t *) &temppc ) );
        cpu->PC = addr;
    } else if ( ( op.instruction_type == INSTRUCTION__RTS ) || (op.instruction_type == INSTRUCTION__RTI) ) {
        if ( ( op.instruction_type == INSTRUCTION__RTI ) ) {
            cpu->P == STACK_PULL();
        }
        *( (uint8_t *) &temppc ) = STACK_PULL();
        *( ((uint8_t *) &temppc) + 1 ) = STACK_PULL();

        if ( op.instruction_type == INSTRUCTION__RTS ) {
            temppc++;
        }

        cpu->PC = temppc;

    } else {
        cpu->PC += op.size;
    }

    if ( sig & CPU_6502_SIGNAL__NMI ) {
        printf("NMI\n");
        temppc = cpu->PC;
        STACK_PUSH( *( ( (uint8_t *) &temppc ) + 1 ) );
        STACK_PUSH( *( (uint8_t *) &temppc ) );
        STACK_PUSH( cpu->P );
        cpu->PC = READ( NMI_VECTOR_LO );
        cpu->PC |= READ( NMI_VECTOR_HI ) << 8;
    }



}

void cpuDumpStack( struct cpu6502 * cpu ) {
    uint16_t i;
    i = 0x01ff;
    while ( i > (0x100 + (uint16_t) cpu->SP) ) {
        printf("[$%04" PRIx16 "] = $%02" PRIx8 "\n",i,READ(i) );
        i--;
    }
}
