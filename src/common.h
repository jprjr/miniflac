/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_COMMON_H
#define MINIFLAC_COMMON_H

typedef enum MINIFLAC_RESULT MINIFLAC_RESULT;

enum MINIFLAC_RESULT {
    MINIFLAC_OGG_HEADER_NOTFLAC                = -18, /* attempted to read an Ogg header packet that isn't a FLAC-in-Ogg packet */
    MINIFLAC_SUBFRAME_RESERVED_TYPE            = -17, /* subframe header specified a reserved type */
    MINIFLAC_SUBFRAME_RESERVED_BIT             = -16, /* subframe header found a non-zero value in the reserved bit */
    MINIFLAC_STREAMMARKER_INVALID              = -15, /* encountered an illegal value while parsing the fLaC stream marker */
    MINIFLAC_RESERVED_CODING_METHOD            = -14, /* a residual block used a reserved coding method */
    MINIFLAC_METADATA_TYPE_RESERVED            = -13, /* a metadata header used a reserved type */
    MINIFLAC_METADATA_TYPE_INVALID             = -12, /* a metadata header used a invalid type */
    MINIFLAC_FRAME_RESERVED_SAMPLE_SIZE        = -11, /* the frame header lists reserved sample size */
    MINIFLAC_FRAME_RESERVED_CHANNEL_ASSIGNMENT = -10, /* the frame header lists reserved channel assignment */
    MINIFLAC_FRAME_INVALID_SAMPLE_SIZE         =  -9, /* the frame header sample rate was invalid */
    MINIFLAC_FRAME_INVALID_SAMPLE_RATE         =  -8, /* the frame header sample rate was invalid */
    MINIFLAC_FRAME_RESERVED_BLOCKSIZE          =  -7, /* the frame header lists a reserved block size */
    MINIFLAC_FRAME_RESERVED_BIT2               =  -6, /* the second reserved bit was non-zero when parsing the frame header */
    MINIFLAC_FRAME_RESERVED_BIT1               =  -5, /* the first reserved bit was non-zero when parsing the frame header */
    MINIFLAC_FRAME_SYNCCODE_INVALID            =  -4, /* error when parsing a header sync code */
    MINIFLAC_FRAME_CRC16_INVALID               =  -3, /* error in crc16 while decoding frame footer */
    MINIFLAC_FRAME_CRC8_INVALID                =  -2, /* error in crc8 while decoding frame header */
    MINIFLAC_ERROR                             =  -1, /* generic error, likely in an invalid state */
    MINIFLAC_CONTINUE                          =   0, /* needs more data, otherwise fine */
    MINIFLAC_OK                                =   1, /* generic "OK" */
    MINIFLAC_METADATA_END                      =   2, /* used to signify end-of-data in a metadata block */
};

#if defined(__GNUC__) && __GNUC__ >= 2 && __GNUC_MINOR__ >= 5
#define MINIFLAC_PURE __attribute__((const))
#endif

#ifndef MINIFLAC_PURE
#define MINIFLAC_PURE
#endif

#ifdef MINIFLAC_ABORT_ON_ERROR
#include <stdlib.h>
#define miniflac_abort() abort()
#else
#define miniflac_abort()
#endif

#define MINIFLAC_API
#define MINIFLAC_PRIVATE

#endif
