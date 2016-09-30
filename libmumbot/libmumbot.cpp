#include "libmumbot.h"



std::string getVint(uint64_t val) { //TODO deal with endianness
    std::string retval;
	D(std::cout << "Encoding vint:" << val << "\n");
    if (val > 0xffffffff) { //over 32 bits, encode as 64
        uint8_t d[9];
		uint64_t bsval = __builtin_bswap64(val);
		uint8_t *tval = (uint8_t *) &bsval;
        d[0] = 0b11110100;
        std::memcpy(d + 1,tval,8);
        retval = std::string((char *)d,9);
    }
    else if (val > 0xfffffff) { //over 28 bits, encode as 32
		uint8_t d[5];
		uint64_t bsval = __builtin_bswap64(val);
		uint8_t *tval = (uint8_t *) &bsval;
		d[0] = 0b11110000;
		std::memcpy(d + 1, tval + 4, 4);
        retval = std::string((char *)d,5);
    }
    else if (val > 0x1FFFFF) //over 21 bits, encode as 28
    {
        uint8_t d[4];
		uint64_t bsval = __builtin_bswap64(val);
		uint8_t *tval = (uint8_t *) &bsval;
		std::memcpy(d, tval + 4, 4);
		d[0] = d[0] | 0b11100000;
        retval = std::string((char *)d,4);
    }
    else if (val > 0x3FFF) //over 14 bytes, encode as 21
    {

		uint8_t d[3];
		uint64_t bsval = __builtin_bswap64(val);
		uint8_t *tval = (uint8_t *) &bsval;
		std::memcpy(d, tval + 5, 3);
		d[0] = d[0] | 0b11000000;
		retval = std::string((char *)d,3);
    }
    else if (val > 0x7F) { //over 7 bits, encode as 14
		uint8_t d[2];
		uint64_t bsval = __builtin_bswap64(val);
		uint8_t *tval = (uint8_t *) &bsval;
		std::memcpy(d, tval + 6, 2);
		d[0] = d[0] | 0b10000000;
		retval = std::string((char *)d,2);
    }
    else if (val > 0) { //still positive, encode as 7 bit

        uint8_t d[1];
		uint8_t *tval = (uint8_t *) &val;
        d[0] = (0b01111111 & tval[0]);
        retval = std::string((char *)d,1);
    }
    return retval;

}
uint64_t readNextVint(std::string &data, uint32_t pos, uint32_t *finishpos) { //TODO deal with endianness
    //todo deal with endianness
    if (pos > data.length()) {
        std::cout << "Error reading varint\n";
        return -1;
    }

    uint8_t vint_type = data[pos];
    uint64_t vint = 0;
	uint8_t *tval = (uint8_t *) &vint;

    if (vint_type >= 0b11111100) { //byte inverted negative two bit number
        D(std::cout << "byte inverted negative 2 bit varint read: " << vint << "\n"); //TODO
    }
    else if (vint_type >= 0b11111000) { //negative recursive varint
        D(std::cout << "negative recursive varint read: " << vint << "\n"); //TODO
    }
    else if (vint_type >= 0b11110100) { //64 bit positive number
		uint8_t d[8];
		std::memcpy(tval,&data[pos] + 1,8);
    	vint = __builtin_bswap64(vint);

        *finishpos = pos + 9;
		D(std::cout << "64 bit varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b11110000) { //32 bit positive number
		uint8_t d[4];
		std::memcpy(tval + 4,&data[pos] + 1,4);
		vint = __builtin_bswap64(vint);
        *finishpos = pos + 5;
		D(std::cout << "32 bit varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b11100000) { //28 bit positive number
		uint8_t d[4];
		std::memcpy(d,&data[pos],4);
		d[0] = d[0] & 0b00001111;
		std::memcpy(tval + 4,d,4);
		vint = __builtin_bswap64(vint);
        *finishpos = pos + 4;
		D(std::cout << "28 bit varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b11000000) { //21 bit positive number
		uint8_t d[3];
		std::memcpy(d,&data[pos],3);
		d[0] = d[0] & 0b00011111;
		std::memcpy(tval + 5,d,3);
		vint = __builtin_bswap64(vint);
        *finishpos = pos + 3;
		D(std::cout << "21 bit varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b10000000) { //14 bit positive number
		uint8_t d[2];
		std::memcpy(d,&data[pos],2);
		d[0] = d[0] & 0b00111111;
		std::memcpy(tval + 6,d,2);
		vint = __builtin_bswap64(vint);
        *finishpos = pos + 2;
		D(std::cout << "14 bit varint read: " << vint << "\n");
    }
    else { //7 bit positive number
        uint64_t d[1] = {0};
        d[0] = ((uint8_t)data[pos] & 0b01111111); //mask off taken bits
        vint = d[0];
        *finishpos = pos + 1;
		D(std::cout << "7 bit varint read: " << vint << "\n");
    }

    return vint;

}
