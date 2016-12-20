#include <inttypes.h>

uint8_t * PRGRomBankLower;
uint8_t * PRGRomBankUpper;
uint8_t * ZeroPage;
uint8_t * Stack;
uint8_t * RAM;

uint8_t mm_read_abs( uint8_t * value , uint16_t address ) {

    long offset;

    if ( address >= 0x0800 && address < 0x2000 ) {
        // mirroring
        address %= 0x0800;
    }

    if ( address >= 0x2008 & address < 0x4000 ) {
        // mirroring
        address = ( address % 0x0008 ) +  0x2000;
    }

    if ( address >= 0x8000 && address < 0XC000 ) {
        offset = address - 0x8000;
        *value = *(PRGRomBankLower + offset);
        return 1;
    }

    if ( address >= 0x0000 && address < 0x0100 ) {
        offset = address - 0x0100;
        *value = *(ZeroPage + offset);
        return 1;
    }

    if ( address >= 0x0100 && address < 0x0200 ) {
        offset = address - 0x0200;
        *value = *(Stack + offset);
        return 1;
    }
    
    if ( address >= 0x0200 && address < 0x0800 ) {
        offset = address - 0x0200;
        *value = *(RAM + offset);
        return 1;
    }

    return 0;

}

uint8_t mm_write_abs( uint8_t value , uint16_t address ) {

    long offset;

    if ( address >= 0x0800 && address < 0x2000 ) {
        // mirroring
        address %= 0x0800;
    }

    if ( address >= 0x2008 & address < 0x4000 ) {
        // mirroring
        address = ( address % 0x0008 ) +  0x2000;
    }

    if ( address >= 0x8000 && address < 0XC000 ) {
        offset = address - 0x8000;
        *(PRGRomBankLower + offset) = value;
        return 1;
    }

    if ( address >= 0x0000 && address < 0x0100 ) {
        offset = address - 0x0100;
        *(ZeroPage + offset) = value;
        return 1;
    }

    if ( address >= 0x0100 && address < 0x0200 ) {
        offset = address - 0x0200;
        *(Stack + offset) = value;
        return 1;
    }
    
    if ( address >= 0x0200 && address < 0x0800 ) {
        offset = address - 0x0200;
        *(RAM + offset) = value;
        return 1;
    }

    return 0;

}

uint8_t mm_read_zp( uint8_t address ) {
    return (ZeroPage + address);
}