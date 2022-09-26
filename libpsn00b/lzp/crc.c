#include <stdint.h>
#include "lzp.h"

void initTable16(uint16_t *table) {

	int i, j;
    uint16_t crc, c;

    for (i=0; i<256; i++) {

        crc = 0;
        c   = (uint16_t) i;

        for (j=0; j<8; j++) {

            if ( (crc ^ c) & 0x0001 )
				crc = ( crc >> 1 ) ^ 0xA001;
            else
				crc =   crc >> 1;

            c = c >> 1;
        }

        table[i] = crc;
    }

}

void initTable32(uint32_t *table) {

	int i,j;
	uint32_t crcVal;

	for(i=0; i<256; i++) {

		crcVal = i;

		for(j=0; j<8; j++) {

			if (crcVal&0x00000001L)
				crcVal = (crcVal>>1)^0xEDB88320L;
			else
				crcVal = crcVal>>1;

		}

		table[i] = crcVal;

	}

}

uint16_t lzCRC16(const void* buff, int bytes, uint16_t crc) {

	int i;
	uint16_t tmp, short_c;
	uint16_t crcTable[256];

	initTable16(crcTable);

	for(i=0; i<bytes; i++) {

		short_c = 0x00ff & (uint16_t)((const uint8_t *)buff)[i];

		tmp =  crc       ^ short_c;
		crc = (crc >> 8) ^ crcTable[tmp&0xff];

	}

    return(crc);

}

uint32_t lzCRC32(const void* buff, int bytes, uint32_t crc) {

	int	i;
	const uint8_t	*byteBuff = (const uint8_t *)buff;
	uint32_t		byte;
	uint32_t		crcTable[256];

    initTable32(crcTable);

	for(i=0; i<bytes; i++) {

		byte = 0x000000ffL&(uint32_t)byteBuff[i];
		crc = (crc>>8)^crcTable[(crc^byte)&0xff];

	}

	return(crc^0xFFFFFFFF);

}
