#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <nesmem.h>

#include "SDL2/SDL.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

#define PPU_ADDR_STATE_HI 0
#define PPU_ADDR_STATE_LO 1

void monitorPPUTransfer( struct nesMemoryMap * mm ) {
    static int x = PPU_ADDR_STATE_HI;

    

}