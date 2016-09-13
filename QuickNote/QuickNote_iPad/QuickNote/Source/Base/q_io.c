#define MODULE_NAME "q_io.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "q_io.h"

struct Filling{
	unsigned char flittle_endian;
	unsigned char fill[3];
};
typedef union uEndianTest{
	struct Filling fvalue;
	unsigned int value;
}EndianTest;

static const EndianTest __Endian_Test__ = { (unsigned int)1 };


static void ConvertEndian32(void* aValue )
{
	unsigned char* p = (unsigned char*)aValue;
	p[0] = p[0] ^ p[3];
	p[3] = p[0] ^ p[3];
	p[0] = p[0] ^ p[3];
	p[1] = p[1] ^ p[2];
	p[2] = p[1] ^ p[2];
	p[1] = p[1] ^ p[2];
}

static void ConvertEndian16(void* aValue )
{
	unsigned char* p = (unsigned char*)aValue;
	p[0] = p[0] ^ p[1];
	p[1] = p[0] ^ p[1];
	p[0] = p[0] ^ p[1];
}

float32 io_read_float32(FILE *stream)
{
	float32 ret;
	//If the Byte Order is not little endian, convert it.
	if (__Endian_Test__.fvalue.flittle_endian == 0) {
		fread(&ret, 1, 4, stream);
		ConvertEndian32(&ret);
	} else {
		fread(&ret, 1, 4, stream);
	}
	return ret;
}

float32 ram_read_float32(ram_stream* ram_stream)
{
	float32 ret;

	//If the Byte Order is not little endian, convert it.
	if (__Endian_Test__.fvalue.flittle_endian == 0) {
		memcpy(&ret, ram_stream->iCurrentPtr, 4);
		ConvertEndian32(&ret);
	} else {
		memcpy(&ret, ram_stream->iCurrentPtr, 4);
	}
	ram_stream->iCurrentPtr += 4;
	ram_stream->iReadCount += 4;
	return ret;
}

int32 io_store_float32(FILE *stream, float32 value)
{
	float32 tmp;
	int32 ret;
	//If the Byte Order is not little endian, convert it.
	if (__Endian_Test__.fvalue.flittle_endian == 0) {
		tmp = value;
		ConvertEndian32(&tmp);
		ret = fwrite(&tmp, 1, 4, stream);
	} else {
		ret = fwrite(&value, 1, 4, stream);
	}
	return ret;
}

int32 ram_read_int32(ram_stream* ram_stream)
{
	int32 ret;

	//If the Byte Order is not little endian, convert it.
	if (__Endian_Test__.fvalue.flittle_endian == 0) {
		memcpy(&ret, ram_stream->iCurrentPtr, 4);
		ConvertEndian32(&ret);
	} else {
		memcpy(&ret, ram_stream->iCurrentPtr, 4);
	}
	ram_stream->iCurrentPtr += 4;
	ram_stream->iReadCount += 4;
	return ret;
}

int32 io_read_int32(FILE *stream)
{
	int32 ret;
	//If the Byte Order is not little endian, convert it.
	if (__Endian_Test__.fvalue.flittle_endian == 0) {
		fread(&ret, 1, 4, stream);
		ConvertEndian32(&ret);
	} else {
		fread(&ret, 1, 4, stream);
	}
	return ret;
}

int32 io_store_int32(FILE *stream, int32 value)
{
	int32 tmp;
	int32 ret;
	//If the Byte Order is not little endian, convert it.
	if (__Endian_Test__.fvalue.flittle_endian == 0) {
		tmp = value;
		ConvertEndian32(&tmp);
		ret = fwrite(&tmp, 1, 4, stream);
	} else {
		ret = fwrite(&value, 1, 4, stream);
	}	

	return ret;
}

int16 ram_read_int16(ram_stream* ram_stream)
{
	int16 ret;
	//If the Byte Order is not little endian, convert it.
	if (__Endian_Test__.fvalue.flittle_endian == 0) {
		memcpy(&ret, ram_stream->iCurrentPtr, 2);
		ConvertEndian16(&ret);
	} else {
		memcpy(&ret, ram_stream->iCurrentPtr, 2);
	}
	ram_stream->iCurrentPtr += 2;
	ram_stream->iReadCount += 2;
	return ret;
}

int8 ram_read_int8(ram_stream* ram_stream)
{
	int8 ret;
	memcpy(&ret, ram_stream->iCurrentPtr, 1);
	ram_stream->iCurrentPtr += 1;
	ram_stream->iReadCount += 1;
	return ret;
}

uint8 ram_read_uint8(ram_stream* ram_stream)
{
	uint8 ret;
	memcpy(&ret, ram_stream->iCurrentPtr, 1);
	ram_stream->iCurrentPtr += 1;
	ram_stream->iReadCount += 1;
	return ret;
}

int16 io_read_int16(FILE *stream)
{
	int16 ret;
	//If the Byte Order is not little endian, convert it.
	if (__Endian_Test__.fvalue.flittle_endian == 0) {
		fread(&ret, 1, 2, stream);
		ConvertEndian16(&ret);
	} else {
		fread(&ret, 1, 2, stream);
	}
	return ret;
}

int32 io_store_int16(FILE *stream, int16 value)
{
	int16 tmp;
	int32 ret;
	//If the Byte Order is not little endian, convert it.
	if (__Endian_Test__.fvalue.flittle_endian == 0) {
		tmp = value;
		ConvertEndian16(&tmp);
		ret = fwrite(&tmp, 1, 2, stream);
	} else {
		ret = fwrite(&value, 1, 2, stream);
	}	
	return ret;
}
