#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "nesmem.h"
#include "2c02.h"

#define INES_HEADER_SIZE 16
#define PRG_ROM_BANK_SIZE 16384 // 16KB 
#define CHR_ROM_BANK_SIZE 8192 // 8KB

#define PPU_ADDR_STATE_HI 0
#define PPU_ADDR_STATE_LO 1

#define DEBUG

uint8_t ppuMem[0x10000] = {0};

void ppuMemWrite( const uint16_t addr , const uint8_t data );

void nesMemDebugFindRegionName(const uint16_t addr, char *name ) {

    if (addr==PPU_CTRL_0) {
        strcpy(name,"PPU_CTRL_0");
    } else if (addr==PPU_CTRL_1) {
        strcpy(name,"PPU_CTRL_1");
    } else if (addr==PPU_STATUS) {
        strcpy(name,"PPU_STATUS");
    } else if (addr==OAM_ADDR) {
        strcpy(name,"OAM_ADDR");
    } else if (addr==OAM_DATA) {
        strcpy(name,"OAM_DATA");
    } else if (addr==PPU_SCROLL) {
        strcpy(name,"PPU_SCROLL");
    } else if (addr==PPU_ADDR) {
        strcpy(name,"PPU_ADDR");
    } else if (addr==PPU_DATA) {
        strcpy(name,"PPU_DATA");
    } else if (addr==OAM_DMA) {
        strcpy(name,"OAM_DMA");
    } else {
        strcpy(name,"OTHER");
    }
    
}

uint8_t testRead( struct nesMemoryMap * mm,  const uint16_t addr ) {
    uint8_t data;

// #ifdef DEBUG

//     char rnm[32]; 
//     nesMemDebugFindRegionName( addr, rnm );

//     printf("R $%" PRIx16 " (%s)\n",addr,rnm);

// #endif

    data = mm->mem[addr];
    return data;
}

void testWrite( struct nesMemoryMap * mm, const uint16_t addr, const uint8_t data ) {

    static int ppu_addr_st = PPU_ADDR_STATE_LO;
    static uint16_t ppu_addr = 0x0;
    static uint16_t ppu_addr_inc = 0;

#ifdef DEBUG

    char rnm[32]; 
    nesMemDebugFindRegionName( addr, rnm );

    printf("W $%" PRIx16 " (%s)\n",addr,rnm);

#endif

    if ( addr == PPU_ADDR ) {
        printf("(PPU_ADDR)");
        ppu_addr_st = !ppu_addr_st;
        if ( ppu_addr_st == PPU_ADDR_STATE_LO ) {
            printf("(LO)");
            ppu_addr |= ( uint16_t ) data;
            ppu_addr_inc = 0;

        } else {
            printf("(HI)");
            ppu_addr = ( ( ( uint16_t ) data ) << 8 );
        }
    } else if ( addr == PPU_DATA ) {
        printf("(PPU_DATA)");
        printf("PPU_CTRL_0: $%02" PRIx8, mm->mem[PPU_CTRL_0] );
        if ( ppu_addr_st == PPU_ADDR_STATE_LO ) {
            ppuMemWrite( ppu_addr + ppu_addr_inc , data );
        }
        ppu_addr_inc++;
    }

    mm->mem[addr] = data;
    return;

}


int nesMemLoadINES( struct nesMemoryMap * mm, const char * fname ) {

    FILE *fp;
    int r,i;
    uint8_t header[INES_HEADER_SIZE];

    fp = fopen(fname,"r");

    if (fp==NULL) {
        return -1;
    }



    r = fread(header,INES_HEADER_SIZE,1,fp);

    if (r!=1) {
        return -2;
    }

    if ( header[0]!='N' || header[1]!='E' || header[2]!='S' || header[3]!=0x1A ) {
        return -3;
    }

    for(i=0;i<2;i++) {
        r = fread( (mm->mem)+0x8000 + i*PRG_ROM_BANK_SIZE , PRG_ROM_BANK_SIZE, 1,fp);
        if (r!=1) {
            return (-3 - i); 
        }
    }

    r = fread( ppuMem, CHR_ROM_BANK_SIZE, 1, fp);

    if (r!=1) {
        return (-3 - i);
    }

}



void nesMemoryMapTestInit(struct nesMemoryMap * mm) {

    memset(mm->mem,0, (sizeof mm->mem) );
    mm->mem[0x2002] = 0x80;
    mm->read = &testRead;
    mm->write = &testWrite;
}

void ppuMemWrite( const uint16_t addr , const uint8_t data ) {

    char name[32];
    int offset;

    ppuMem[addr] = data;
    

#ifdef DEBUG
    
    if ( ( addr - PATTERN_TABLE_0 ) < PATTERN_TABLE_LENGTH ) {
        offset = ( addr - PATTERN_TABLE_0 );
        strcpy(name,"PATTERN_TABLE_0");
    } else if ( ( addr - PATTERN_TABLE_1 ) < PATTERN_TABLE_LENGTH ) {
        offset = ( addr - PATTERN_TABLE_1 );
        strcpy(name,"PATTERN_TABLE_1");
    } else if ( ( addr - NAME_TABLE_0 ) < NAME_TABLE_LENGTH ) {
        offset = ( addr - NAME_TABLE_0 );
        strcpy(name,"NAME_TABLE_0");
    } else if ( ( addr - ATTR_TABLE_0 ) < ATTR_TABLE_LENGTH ) {
        offset = ( addr - ATTR_TABLE_0 );
        strcpy(name,"ATTR_TABLE_0");
    } else if ( ( addr - NAME_TABLE_1 ) < NAME_TABLE_LENGTH ) {
        offset = ( addr - NAME_TABLE_1 );
        strcpy(name,"NAME_TABLE_1");
    } else if ( ( addr - ATTR_TABLE_1 ) < ATTR_TABLE_LENGTH ) {
        offset = ( addr - ATTR_TABLE_1 );
        strcpy(name,"ATTR_TABLE_1");
    } else if ( ( addr - NAME_TABLE_2 ) < NAME_TABLE_LENGTH ) {
        offset = ( addr - NAME_TABLE_2 );
        strcpy(name,"NAME_TABLE_2");
    } else if ( ( addr - ATTR_TABLE_2 ) < ATTR_TABLE_LENGTH ) {
        offset = ( addr - ATTR_TABLE_2 );
        strcpy(name,"ATTR_TABLE_2");
    } else if ( ( addr - NAME_TABLE_3 ) < NAME_TABLE_LENGTH ) {
        offset = ( addr - NAME_TABLE_3 );
        strcpy(name,"NAME_TABLE_3");
    } else if ( ( addr - ATTR_TABLE_3 ) < ATTR_TABLE_LENGTH ) {
        offset = ( addr - ATTR_TABLE_3 );
        strcpy(name,"ATTR_TABLE_3");
    } else if ( ( addr - IMAGE_PALETTE ) < PALETTE_LENGTH ) {
        offset = ( addr - IMAGE_PALETTE );
        strcpy(name,"IMAGE_PALETTE");
    } else if ( ( addr - SPRITE_PALETTE ) < PALETTE_LENGTH ) {
        offset = ( addr - SPRITE_PALETTE );
        strcpy(name,"SPRITE_PALETTE"); 
    } else {
        offset = 0;
        strcpy(name,"OTHER");
    }

    printf("PPU MEM W: [$%02" PRIx16 "]=$%02" PRIx8 " %s + $%02x", addr, data, name, offset);

#endif /* DEBUG */

}

void ppuMemDumpNameTable(const int nt) {
    int x,y;
    uint16_t base;
    uint8_t a;

    base = NAME_TABLE_0 + 0x400*nt;
    for (y=0;y<30;y++) {
        for (x=0;x<32;x++) {
            a = ppuMem[base + 32*y + x];
            printf("%02" PRIx8 " ",a);
        }
        printf("\n");
    }
}

void cpuMemDumpPage( struct nesMemoryMap * mm, const int p) {
    int i,j;
    uint8_t a;

    for(i=0;i<16;i++) {
        printf("$%02x ",i);
        for(j=0;j<16;j++) {
            a = mm->mem[p + i*16 + j];
            printf("%02" PRIx8 " ",a);
        }
        printf("\n");
    }
}

void cpuMemDumpNonZero( struct nesMemoryMap *mm ) {
    uint16_t i;
    uint8_t v;

    for(i=0;i<0x8000;i++) {
        v = mm->mem[i];
        if( v != 0 ) printf("[$%04" PRIx16 "] = $%02" PRIx8 "\n", i,v);
    }
}

