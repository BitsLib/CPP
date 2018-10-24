#include "BitsLibc.h"

static unsigned int  BigLittleEndian = 0x00000001;
#define IS_LITTLE_ENDIAN()  (((uint8_t*)(&BigLittleEndian))[0]>0)

uint64_t Bits2UInt64(uint8_t *bits, int idx, int cnt)
{
	assert((cnt > 0) && (cnt <= 56) && "cnt must in  (0,56]");
	uint64_t val = 0;
	uint8_t* pval = (uint8_t*)(&val);
	int startIdx = idx >> 3;
	int endIdx = (int)(ceil((idx + cnt) / 8.0));
	int byteSize = endIdx - startIdx;
	if (IS_LITTLE_ENDIAN())
	{
		for (int i = 0; i < byteSize; i++)
		{
			pval[7 - i] = bits[startIdx + i];
		}
	}
	else
	{
		for (int i = 0; i < byteSize; i++)
		{
			pval[i] = bits[startIdx + i];
		}
	}
	val = val << (idx % 8);
	val = val >> (64 - cnt);
	return val;
}

uint32_t Bits2UInt32(uint8_t *bits, int idx, int cnt)
{
	assert((cnt > 0) && (cnt <= 32) && "cnt must in  (0,32]");
	return (uint32_t)(0x00000000FFFFFFFF & Bits2UInt64(bits,idx,cnt));
}

uint16_t Bits2UInt16(uint8_t*bits, int idx, int cnt)
{
	assert((cnt > 0) && (cnt <= 16) && "cnt must in  (0,16]");
	uint8_t * buf = (uint8_t *)bits;
	uint32_t val = 0;
	uint8_t* pval = (uint8_t*)(&val);
	int startIdx = idx >> 3;
	int endIdx = (int)(ceil((idx + cnt) / 8.0));
	int byteSize = endIdx - startIdx;
	if (IS_LITTLE_ENDIAN())
	{
		for (int i = 0; i<byteSize; i++)
		{
			pval[3 - i] = buf[startIdx + i];
		}
	}
	else
	{
		for (int i = 0; i<byteSize; i++)
		{
			pval[i] = buf[startIdx + i];
		}
	}
	val = val << (idx % 8);
	val = val >> (32 - cnt);
	return (uint16_t)(0x0000FFFF & val);
}

uint8_t Bits2UInt8(uint8_t*bits, int idx, int cnt)
{
	assert((cnt > 0) && (cnt <= 8) && "cnt must in  (0,8]");
	return (uint8_t)(0x00FF & Bits2UInt16(bits, idx, cnt));
}

void BitsCopyTo(uint8_t*bits, int idx, int cnt, uint64_t value)
{
	assert((cnt > 0) && (cnt <= 56) && "cnt must in  (0,56]");
	uint64_t val = value & 0x00FFFFFFFFFFFFFF;
	val = (val << (64 - cnt));
	val = (val >> (idx % 8));
	uint8_t* pval = (uint8_t*)(&val);
	int startIdx = idx >> 3;
	int endIdx = (int)(ceil((idx + cnt) / 8.0));
	int byteSize = endIdx - startIdx;

	bits[startIdx] &= (0xFF << (8 - idx % 8));
	if (byteSize >= 2)
	{
		memset(bits + startIdx + 1, 0, byteSize - 2);
		bits[endIdx - 1] &= (0xFF >> (8 - (idx + cnt) % 8));
	}

	if (IS_LITTLE_ENDIAN())
	{
		for (int i = 0; i<byteSize; i++)
		{
			bits[startIdx + i] |= pval[7 - i];
		}
	}
	else
	{
		for (int i = 0; i<byteSize; i++)
		{
			bits[startIdx + i] |= pval[i];
		}
	}
}
uint32_t CheckCrc(uint8_t *bits, int length)
{
	// CRC values for all single-byte messages; used to speed up CRC calculation.
	static uint32_t crc_table[] = {
		0x3935ea, 0x1c9af5, 0xf1b77e, 0x78dbbf, 0xc397db, 0x9e31e9, 0xb0e2f0, 0x587178,
		0x2c38bc, 0x161c5e, 0x0b0e2f, 0xfa7d13, 0x82c48d, 0xbe9842, 0x5f4c21, 0xd05c14,
		0x682e0a, 0x341705, 0xe5f186, 0x72f8c3, 0xc68665, 0x9cb936, 0x4e5c9b, 0xd8d449,
		0x939020, 0x49c810, 0x24e408, 0x127204, 0x093902, 0x049c81, 0xfdb444, 0x7eda22,
		0x3f6d11, 0xe04c8c, 0x702646, 0x381323, 0xe3f395, 0x8e03ce, 0x4701e7, 0xdc7af7,
		0x91c77f, 0xb719bb, 0xa476d9, 0xadc168, 0x56e0b4, 0x2b705a, 0x15b82d, 0xf52612,
		0x7a9309, 0xc2b380, 0x6159c0, 0x30ace0, 0x185670, 0x0c2b38, 0x06159c, 0x030ace,
		0x018567, 0xff38b7, 0x80665f, 0xbfc92b, 0xa01e91, 0xaff54c, 0x57faa6, 0x2bfd53,
		0xea04ad, 0x8af852, 0x457c29, 0xdd4410, 0x6ea208, 0x375104, 0x1ba882, 0x0dd441,
		0xf91024, 0x7c8812, 0x3e4409, 0xe0d800, 0x706c00, 0x383600, 0x1c1b00, 0x0e0d80,
		0x0706c0, 0x038360, 0x01c1b0, 0x00e0d8, 0x00706c, 0x003836, 0x001c1b, 0xfff409,
		0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
		0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
		0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000
	};
	uint32_t crc = 0;
	int offset = (length == 112) ? 0 : (112 - 56);
	for (int j = 0; j < length; j++)
	{
		// If bit is set, xor with corresponding table entry.
		if (bits[(j / 8)] & (1 << (7 - (j % 8))))
			crc ^= crc_table[j + offset];
	}
	return crc; // 24 bit checksum.

}


