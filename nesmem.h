#ifndef __NESMEM_H
#define __NESMEM_H

#include <stdint.h>

#define NES_MEM_SIZE 0x10000

typedef int nesMemErr;

struct nesMemoryMap {
    uint8_t mem[NES_MEM_SIZE];
    uint8_t (*read)( struct nesMemoryMap *, uint16_t );
    void (*write)( struct nesMemoryMap *, uint16_t, uint8_t );
};


int nesMemLoadINES( struct nesMemoryMap * mm, const char * fname );

void nesMemoryMapTestInit( struct nesMemoryMap * mm);

void ppuMemDumpNameTable(const int nt);

void cpuMemDumpPage( struct nesMemoryMap *mm, const int p);

void cpuMemDumpNonZero( struct nesMemoryMap *mm ) ;

#endif /* __NESMEM_H */