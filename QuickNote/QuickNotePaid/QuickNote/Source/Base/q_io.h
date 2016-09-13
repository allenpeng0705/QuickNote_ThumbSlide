#ifndef _QI_IO_H_
#define _QI_IO_H_

#include "base.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct ram_stream {
	unsigned char* iBuffer;
	unsigned char* iCurrentPtr;
	uint32 iReadCount;
} ram_stream;

/* 
 * Reads four bytes (byte-per-byte) and reconstructs
 * a float32 value.
 */
float32 io_read_float32(FILE *stream);

float32 ram_read_float32(ram_stream* ram_stream);

/* 
 * Writes out a float32 value as four bytes (byte-per-byte) in a CPU-independent
 * representation.
 */
int32 io_store_float32(FILE *stream, float32 value);

/* Reads four bytes (byte-per-byte) and reconstructs
 * an int32 value.
 */
int32 io_read_int32(FILE *stream);

int32 ram_read_int32(ram_stream* ram_stream);

/* Writes out an int32 value as four bytes (byte-per-byte) in a CPU-independent
 * representation.
 */
int32 io_store_int32(FILE *stream, int32 value);

/* Reads four bytes (byte-per-byte) and reconstructs
 * an int16 value.
 */
int16 io_read_int16(FILE *stream);

int16 ram_read_int16(ram_stream* ram_stream);

/* Writes out an int16 value as four bytes (byte-per-byte) in a CPU-independent
 * representation.
 */
int32 io_store_int16(FILE *stream, int16 value);

int8 ram_read_int8(ram_stream* ram_stream);
uint8 ram_read_uint8(ram_stream* ram_stream);


#ifdef __cplusplus
}
#endif

#endif