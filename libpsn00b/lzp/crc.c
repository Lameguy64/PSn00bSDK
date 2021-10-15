#include "lzp.h"

void initTable16(unsigned short* table) {

	int i, j;
    unsigned short crc, c;

    for (i=0; i<256; i++) {

        crc = 0;
        c   = (unsigned short) i;

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

void initTable32(unsigned int* table) {

	int i,j;
	unsigned int crcVal;

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

unsigned short lzCRC16(const void* buff, int bytes, unsigned short crc) {

	int i;
	unsigned short tmp, short_c;
	unsigned short crcTable[256];

	initTable16(crcTable);

	for(i=0; i<bytes; i++) {

		short_c = 0x00ff & (unsigned short)((const unsigned char*)buff)[i];

		tmp =  crc       ^ short_c;
		crc = (crc >> 8) ^ crcTable[tmp&0xff];

	}

    return(crc);

}

unsigned int lzCRC32(const void* buff, int bytes, unsigned int crc) {

	int	i;
	const unsigned char*	byteBuff = (const unsigned char*)buff;
	unsigned int			byte;
	unsigned int			crcTable[256];

    initTable32(crcTable);

	for(i=0; i<bytes; i++) {

		byte = 0x000000ffL&(unsigned int)byteBuff[i];
		crc = (crc>>8)^crcTable[(crc^byte)&0xff];

	}

	return(crc^0xFFFFFFFF);

}
