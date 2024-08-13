/*
 * sm_utils.h
 *
 *  Created on: Jun 29, 2019
 *      Author: quangnd
 */

#ifndef SM_MEMORY_H
#define SM_MEMORY_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define SM_LITTLE_ENDIAN

#ifndef mem_alloc
#define mem_alloc    malloc
#endif

#ifndef mem_free
#define mem_free     free
#endif

#ifndef memcpy
#define memcpy      memcpy
#endif

#ifndef memset
#define memset      memset
#endif

#ifndef memcmp
#define memcmp      memcmp
#endif

/**
 * Helper union for manipulating data bytes.
 */
typedef union{
    uint8_t  u8[8];  /**< 8 bytes */
    uint16_t u16[4]; /**< 4 words */
    uint32_t u32[2]; /**< 2 double words */
}sm_bytes_t;

/* Helper functions. **********************************************************/
#ifndef memcpy
static inline void memcpy(uint8_t dest[], const uint8_t src[], const uint16_t size){
    uint16_t i;
    for(i = 0; i < size; i++){
        dest[i] = src[i];
    }
}
#endif

/* Return 1: same, return 0: difference */
#ifndef memcmp
static inline bool memcmp(const uint8_t* buff1, const uint8_t* buff2, uint32_t len)
{
	uint32_t i = 0;
	for(; i<len; i++)
	{
		if(buff1[i] != buff2[i]) return 0;
	}
	return 1;
}
#endif

/* Helper functions. **********************************************************/
#ifndef memset
static inline void memset(uint8_t dest[], unsigned char value, const uint16_t size){
    uint16_t i;
    for(i = 0; i < size; i++){
        dest[i] = value;
    }
}
#endif
/**
 * @brief
 * @param data
 * @return
 */
static inline uint16_t sm_getUint16(const uint8_t data[]){
    sm_bytes_t b;
    b.u8[0] = data[1];
    b.u8[1] = data[0];
    return b.u16[0];
}
/**
 * @brief
 * @param data
 * @return
 */
static inline uint32_t sm_getUint32(const uint8_t data[]){
    sm_bytes_t b;
    b.u8[0] = data[3];
    b.u8[1] = data[2];
    b.u8[2] = data[1];
    b.u8[3] = data[0];
    return b.u32[0];
}
/**
 * @brief
 * @param data
 * @param value
 */
static inline void sm_setUint16(uint8_t data[], const uint16_t value){
    sm_bytes_t b;
    b.u16[0] = value;
    data[0] = b.u8[1];
    data[1] = b.u8[0];
}
/**
 * @brief
 * @param data
 * @param value
 */
static inline void sm_setUint32(uint8_t data[], const uint32_t value){
    sm_bytes_t b;
    b.u32[0] = value;
    data[0] = b.u8[0];
    data[1] = b.u8[1];
    data[2] = b.u8[2];
    data[3] = b.u8[3];
}

#ifdef SM_LITTLE_ENDIAN
/**
 * @brief
 * @param dest
 * @param src
 */
static inline void sm_memcpySwap2(void* dest, const void* src){
    char *cdest;
    char *csrc;
    cdest = (char *) dest;
    csrc = (char *) src;
    cdest[0] = csrc[0];
    cdest[1] = csrc[1];
}
/**
 * @brief
 * @param dest
 * @param src
 */
static inline void sm_memcpySwap4(void* dest, const void* src){
    char *cdest;
    char *csrc;
    cdest = (char *) dest;
    csrc = (char *) src;
    cdest[0] = csrc[0];
    cdest[1] = csrc[1];
    cdest[2] = csrc[2];
    cdest[3] = csrc[3];
}
/**
 * @brief
 * @param dest
 * @param src
 */
static inline void sm_memcpySwap8(void* dest, const void* src){
    char *cdest;
    char *csrc;
    cdest = (char *) dest;
    csrc = (char *) src;
    cdest[0] = csrc[0];
    cdest[1] = csrc[1];
    cdest[2] = csrc[2];
    cdest[3] = csrc[3];
    cdest[4] = csrc[4];
    cdest[5] = csrc[5];
    cdest[6] = csrc[6];
    cdest[7] = csrc[7];
}
#endif

#ifdef sm_BIG_ENDIAN
static inline void sm_memcpySwap2(void* dest, const void* src){
    char *cdest;
    char *csrc;
    cdest = (char *) dest;
    csrc = (char *) src;
    cdest[0] = csrc[1];
    cdest[1] = csrc[0];
}

static inline void sm_memcpySwap4(void* dest, const void* src){
    char *cdest;
    char *csrc;
    cdest = (char *) dest;
    csrc = (char *) src;
    cdest[0] = csrc[3];
    cdest[1] = csrc[2];
    cdest[2] = csrc[1];
    cdest[3] = csrc[0];
}

static inline void sm_memcpySwap8(void* dest, const void* src){
    char *cdest;
    char *csrc;
    cdest = (char *) dest;
    csrc = (char *) src;
    cdest[0] = csrc[7];
    cdest[1] = csrc[6];
    cdest[2] = csrc[5];
    cdest[3] = csrc[4];
    cdest[4] = csrc[3];
    cdest[5] = csrc[2];
    cdest[6] = csrc[1];
    cdest[7] = csrc[0];
}
#endif

/**
 * Helper union for manipulating data bits.
 */

// Read Bit at any position
#define sm_READ_BIT(buff, pos)                  ((buff>>pos) & 0x01)

// Write Bit at any position
#define sm_WRITE_BIT(buff, val, pos)            (buff &= ~(1<<pos)); buff |= (val<<pos)


typedef union
{
	uint8_t w;
      struct
	{
		    uint8_t    b0		:1;
		    uint8_t    b1		:1;
		    uint8_t    b2		:1;
		    uint8_t    b3		:1;
		    uint8_t    b4		:1;
		    uint8_t    b5		:1;
		    uint8_t    b6		:1;
		    uint8_t    b7		:1;
	};
} sm_seg_uint8_t;

typedef union
{
	uint32_t w;
      struct
	{
		sm_seg_uint8_t    byte0	;
		sm_seg_uint8_t    byte1	;
		sm_seg_uint8_t    byte2	;
		sm_seg_uint8_t    byte3	;
	};
} sm_seg_uint32_t;
/**
 * @brief
 * @param input
 * @return
 */
static inline uint32_t sm_rounding_divisible_8 (uint32_t input)	//uncheck
{
	uint32_t temp = (input >> 3) * 8;
	return ( temp == input) ? input : (temp + 8);
}
#endif /* SM_MEMORY_H */
