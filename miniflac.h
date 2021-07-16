/* SPDX-License-Identifier: 0BSD
Copyright (C) 2021 John Regan <john@jrjrtech.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/
#ifndef MINIFLAC_H
#define MINIFLAC_H

/*

Building
========

In one C file, define MINIFLAC_IMPLEMENTATION before including this header, like:
#define MINIFLAC_IMPLEMENTATION
#include "miniflac.h"

Usage
=====

You'll want to allocate a decoder, initialize it with miniflac_init, then start
pushing data into it with functions like miniflac_sync or miniflac_decode.

All the "feeding" functions take the same first 4 parameters:
  1. miniflac* pFlac - a pointer to a miniflac struct.
  2. const uint8_t* data - a pointer to a stream of bytes to push.
  3. uint32_t length - the length of your stream of bytes.
  4. uint32_t* out_length - an out-variable, it will be updated with the
                            number of bytes consumed.
You'll to save any un-consumed bytes for your next call.

All the "feeding" / public functions return a MINIFLAC_RESULT enum, these are the
important values:
  < 0 : error
  0 : more data is required (MINIFLAC_CONTINUE)
  1 : success (MINIFLAC_OK)

You can use miniflac_sync to sync to a block boundary. It will automatically parse
a metadata block header or frame header, so you can inspect it and check on things
like the block size, bits-per-sample, etc. See the various struct definitions down
below. The example program in "examples/basic-decoder.c" does this. Keep calling
it until you get something other than MINIFLAC_CONTINUE (0). MINIFLAC_OK (1) means
you've found a block boundary. Anything < 0 is an error.

You can look at the .state field in the miniflac struct to see if you're in a
metadata block or audio frame, and proceed accordingly.

Decoding audio is done with miniflac_decode. You don't *have* to use miniflac_sync first,
you could just call miniflac_decode (this is done in the example program
"examples/single-byte-decoder.c"). The advantage of miniflac_sync is you can
look at the frame properties in the header before you start decoding.

miniflac_decode behaves similarly to miniflac_sync - when it returns MINIFLAC_OK (1),
your output buffer will have decoded audio samples in it. You'll be at the end
of the audio frame, so you can either call miniflac_sync to check the header of the next
frame, or call miniflac_decode to continue on.

*/

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#if !defined(MINIFLAC_API)
    #ifdef MINIFLAC_DLL
        #ifdef _WIN32
            #define MINIFLAC_DLL_IMPORT  __declspec(dllimport)
            #define MINIFLAC_DLL_EXPORT  __declspec(dllexport)
            #define MINIFLAC_DLL_PRIVATE static
        #else
            #if defined(__GNUC__) && __GNUC__ >= 4
                #define MINIFLAC_DLL_IMPORT  __attribute__((visibility("default")))
                #define MINIFLAC_DLL_EXPORT  __attribute__((visibility("default")))
                #define MINIFLAC_DLL_PRIVATE __attribute__((visibility("hidden")))
            #else
                #define MINIFLAC_DLL_IMPORT
                #define MINIFLAC_DLL_EXPORT
                #define MINIFLAC_DLL_PRIVATE static
            #endif
        #endif

        #ifdef MINIFLAC_IMPLEMENTATION
            #define MINIFLAC_API  MINIFLAC_DLL_EXPORT
        #else
            #define MINIFLAC_API  MINIFLAC_DLL_IMPORT
        #endif
        #define MINIFLAC_PRIVATE MINIFLAC_DLL_PRIVATE
    #else
        #define MINIFLAC_API extern
        #define MINIFLAC_PRIVATE static
    #endif
#endif

#if defined(__GNUC__) && __GNUC__ >= 2 && __GNUC_MINOR__ >= 5
#define MINIFLAC_PURE __attribute__((const))
#endif

#define MINIFLAC_COMMON_H
#define MINIFLAC_BITREADER_H
#define MINIFLAC_FRAME_H
#define MINIFLAC_FRAMEHEADER_H
#define MINIFLAC_METADATA_H
#define MINIFLAC_METADATA_HEADER_H
#define MINIFLAC_OGG_H
#define MINIFLAC_OGGHEADER_H
#define MINIFLAC_RESIDUAL_H
#define MINIFLAC_STREAMINFO_H
#define MINIFLAC_STREAMMARKER_H
#define MINIFLAC_SUBFRAME_CONSTANT_H
#define MINIFLAC_SUBFRAME_FIXED_H
#define MINIFLAC_SUBFRAME_H
#define MINIFLAC_SUBFRAME_HEADER_H
#define MINIFLAC_SUBFRAME_LPC_H
#define MINIFLAC_SUBFRAME_VERBATIM_H
#define MINIFLAC_UNPACK_H

#ifndef MINIFLAC_PURE
#define MINIFLAC_PURE
#endif

typedef struct miniflac_bitreader_s miniflac_bitreader;
typedef struct miniflac_oggheader_s miniflac_oggheader_t;
typedef struct miniflac_ogg_s miniflac_ogg_t;
typedef struct miniflac_streammarker_s miniflac_streammarker_t;
typedef struct miniflac_metadata_header_s miniflac_metadata_header;
typedef struct miniflac_streaminfo_s miniflac_streaminfo_t;
typedef struct miniflac_streaminfo_private_s miniflac_streaminfo_private_t;
typedef struct miniflac_vorbiscomment_s miniflac_vorbiscomment_t;
typedef struct miniflac_metadata_s miniflac_metadata;
typedef struct miniflac_residual_s miniflac_residual;
typedef struct miniflac_subframe_fixed_s miniflac_subframe_fixed;
typedef struct miniflac_subframe_lpc_s miniflac_subframe_lpc;
typedef struct miniflac_subframe_constant_s miniflac_subframe_constant;
typedef struct miniflac_subframe_verbatim_s miniflac_subframe_verbatim;
typedef struct miniflac_subframe_header_s miniflac_subframe_header;
typedef struct miniflac_subframe_s miniflac_subframe;
typedef struct miniflac_frame_header_s miniflac_frame_header;
typedef struct miniflac_frame_s miniflac_frame;
typedef struct miniflac_s miniflac_t;

typedef enum MINIFLAC_RESULT MINIFLAC_RESULT;
typedef enum MINIFLAC_OGGHEADER_STATE MINIFLAC_OGGHEADER_STATE;
typedef enum MINIFLAC_OGG_STATE MINIFLAC_OGG_STATE;
typedef enum MINIFLAC_STREAMMARKER_STATE MINIFLAC_STREAMMARKER_STATE;
typedef enum MINIFLAC_METADATA_TYPE MINIFLAC_METADATA_TYPE;
typedef enum MINIFLAC_METADATA_HEADER_STATE MINIFLAC_METADATA_HEADER_STATE;
typedef enum MINIFLAC_STREAMINFO_STATE MINIFLAC_STREAMINFO_STATE;
typedef enum MINIFLAC_VORBISCOMMENT_STATE MINIFLAC_VORBISCOMMENT_STATE;
typedef enum MINIFLAC_METADATA_STATE MINIFLAC_METADATA_STATE;
typedef enum MINIFLAC_RESIDUAL_STATE MINIFLAC_RESIDUAL_STATE;
typedef enum MINIFLAC_SUBFRAME_FIXED_STATE MINIFLAC_SUBFRAME_FIXED_STATE;
typedef enum MINIFLAC_SUBFRAME_LPC_STATE MINIFLAC_SUBFRAME_LPC_STATE;
typedef enum MINIFLAC_SUBFRAME_CONSTANT_STATE MINIFLAC_SUBFRAME_CONSTANT_STATE;
typedef enum MINIFLAC_SUBFRAME_VERBATIM_STATE MINIFLAC_SUBFRAME_VERBATIM_STATE;
typedef enum MINIFLAC_SUBFRAME_TYPE MINIFLAC_SUBFRAME_TYPE;
typedef enum MINIFLAC_SUBFRAME_HEADER_STATE MINIFLAC_SUBFRAME_HEADER_STATE;
typedef enum MINIFLAC_SUBFRAME_STATE MINIFLAC_SUBFRAME_STATE;
typedef enum MINIFLAC_CHASSGN MINIFLAC_CHASSGN;
typedef enum MINIFLAC_FRAME_HEADER_STATE MINIFLAC_FRAME_HEADER_STATE;
typedef enum MINIFLAC_FRAME_STATE MINIFLAC_FRAME_STATE;
typedef enum MINIFLAC_STATE MINIFLAC_STATE;
typedef enum MINIFLAC_CONTAINER MINIFLAC_CONTAINER;

enum MINIFLAC_RESULT {
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
    MINIFLAC_ITERATOR_END                      =   2, /* used by iterators to signify end-of-data */
};

enum MINIFLAC_OGGHEADER_STATE {
    MINIFLAC_OGGHEADER_PACKETTYPE,
    MINIFLAC_OGGHEADER_F,
    MINIFLAC_OGGHEADER_L,
    MINIFLAC_OGGHEADER_A,
    MINIFLAC_OGGHEADER_C,
    MINIFLAC_OGGHEADER_MAJOR,
    MINIFLAC_OGGHEADER_MINOR,
    MINIFLAC_OGGHEADER_HEADERPACKETS,
};

enum MINIFLAC_OGG_STATE {
    MINIFLAC_OGG_CAPTUREPATTERN_O,
    MINIFLAC_OGG_CAPTUREPATTERN_G1,
    MINIFLAC_OGG_CAPTUREPATTERN_G2,
    MINIFLAC_OGG_CAPTUREPATTERN_S,
    MINIFLAC_OGG_VERSION,
    MINIFLAC_OGG_HEADERTYPE,
    MINIFLAC_OGG_GRANULEPOS,
    MINIFLAC_OGG_SERIALNO,
    MINIFLAC_OGG_PAGENO,
    MINIFLAC_OGG_CHECKSUM,
    MINIFLAC_OGG_PAGESEGMENTS,
    MINIFLAC_OGG_SEGMENTTABLE,
    MINIFLAC_OGG_DATA,
};

enum MINIFLAC_STREAMMARKER_STATE {
    MINIFLAC_STREAMMARKER_F,
    MINIFLAC_STREAMMARKER_L,
    MINIFLAC_STREAMMARKER_A,
    MINIFLAC_STREAMMARKER_C,
};

enum MINIFLAC_METADATA_TYPE {
    MINIFLAC_METADATA_UNKNOWN,
    MINIFLAC_METADATA_STREAMINFO,
    MINIFLAC_METADATA_PADDING,
    MINIFLAC_METADATA_APPLICATION,
    MINIFLAC_METADATA_SEEKTABLE,
    MINIFLAC_METADATA_VORBIS_COMMENT,
    MINIFLAC_METADATA_CUESHEET,
    MINIFLAC_METADATA_PICTURE,
    MINIFLAC_METADATA_INVALID,
};

enum MINIFLAC_METADATA_HEADER_STATE {
    MINIFLAC_METADATA_LAST_FLAG,
    MINIFLAC_METADATA_BLOCK_TYPE,
    MINIFLAC_METADATA_LENGTH,
};

enum MINIFLAC_VORBISCOMMENT_STATE {
    MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH,
    MINIFLAC_VORBISCOMMENT_VENDOR_STRING,
    MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS,
    MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH,
    MINIFLAC_VORBISCOMMENT_COMMENT_STRING,
};

enum MINIFLAC_METADATA_STATE {
    MINIFLAC_METADATA_HEADER,
    MINIFLAC_METADATA_DATA,
};

enum MINIFLAC_RESIDUAL_STATE {
    MINIFLAC_RESIDUAL_CODING_METHOD,
    MINIFLAC_RESIDUAL_PARTITION_ORDER,
    MINIFLAC_RESIDUAL_RICE_PARAMETER,
    MINIFLAC_RESIDUAL_RICE_SIZE, /* used when rice_parameter is an escape code */
    MINIFLAC_RESIDUAL_RICE_VALUE, /* used when rice_parameter is an escape code */
    MINIFLAC_RESIDUAL_MSB, /* used when reading MSB bits */
    MINIFLAC_RESIDUAL_LSB, /* used when reading MSB bits */
};

enum MINIFLAC_SUBFRAME_FIXED_STATE {
    MINIFLAC_SUBFRAME_FIXED_DECODE,
};

enum MINIFLAC_SUBFRAME_LPC_STATE {
    MINIFLAC_SUBFRAME_LPC_PRECISION,
    MINIFLAC_SUBFRAME_LPC_SHIFT,
    MINIFLAC_SUBFRAME_LPC_COEFF,
};

enum MINIFLAC_SUBFRAME_CONSTANT_STATE {
    MINIFLAC_SUBFRAME_CONSTANT_DECODE,
};

enum MINIFLAC_SUBFRAME_VERBATIM_STATE {
    MINIFLAC_SUBFRAME_VERBATIM_DECODE,
};

enum MINIFLAC_SUBFRAME_TYPE {
    MINIFLAC_SUBFRAME_TYPE_UNKNOWN,
    MINIFLAC_SUBFRAME_TYPE_CONSTANT,
    MINIFLAC_SUBFRAME_TYPE_FIXED,
    MINIFLAC_SUBFRAME_TYPE_LPC,
    MINIFLAC_SUBFRAME_TYPE_VERBATIM,
};

enum MINIFLAC_SUBFRAME_HEADER_STATE {
    MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1,
    MINIFLAC_SUBFRAME_HEADER_KIND,
    MINIFLAC_SUBFRAME_HEADER_WASTED_BITS,
    MINIFLAC_SUBFRAME_HEADER_UNARY,
};

enum MINIFLAC_SUBFRAME_STATE {
    MINIFLAC_SUBFRAME_HEADER,
    MINIFLAC_SUBFRAME_CONSTANT,
    MINIFLAC_SUBFRAME_VERBATIM,
    MINIFLAC_SUBFRAME_FIXED,
    MINIFLAC_SUBFRAME_LPC,
};

enum MINIFLAC_CHASSGN {
    MINIFLAC_CHASSGN_NONE,
    MINIFLAC_CHASSGN_LEFT_SIDE,
    MINIFLAC_CHASSGN_RIGHT_SIDE,
    MINIFLAC_CHASSGN_MID_SIDE,
};

enum MINIFLAC_FRAME_HEADER_STATE {
    MINIFLAC_FRAME_HEADER_SYNC,
    MINIFLAC_FRAME_HEADER_RESERVEBIT_1,
    MINIFLAC_FRAME_HEADER_BLOCKINGSTRATEGY,
    MINIFLAC_FRAME_HEADER_BLOCKSIZE,
    MINIFLAC_FRAME_HEADER_SAMPLERATE,
    MINIFLAC_FRAME_HEADER_CHANNELASSIGNMENT,
    MINIFLAC_FRAME_HEADER_SAMPLESIZE,
    MINIFLAC_FRAME_HEADER_RESERVEBIT_2,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_1,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_2,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_3,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_4,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_5,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_6,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_7,
    MINIFLAC_FRAME_HEADER_BLOCKSIZE_MAYBE,
    MINIFLAC_FRAME_HEADER_SAMPLERATE_MAYBE,
    MINIFLAC_FRAME_HEADER_CRC8,
};

enum MINIFLAC_FRAME_STATE {
    MINIFLAC_FRAME_HEADER,
    MINIFLAC_FRAME_SUBFRAME,
    MINIFLAC_FRAME_FOOTER,
};

enum MINIFLAC_STATE {
    MINIFLAC_OGGHEADER, /* will try to find an ogg header */
    MINIFLAC_STREAMMARKER_OR_FRAME, /* poke for a stream marker or audio frame
    */
    MINIFLAC_STREAMMARKER, /* reading a stream marker */
    MINIFLAC_METADATA_OR_FRAME, /* will try to find a frame sync code or try to parse a metadata block */
    MINIFLAC_METADATA, /* currently reading a metadata block */
    MINIFLAC_FRAME,    /* currently reading an audio frame */
};

enum MINIFLAC_CONTAINER {
    MINIFLAC_CONTAINER_UNKNOWN,
    MINIFLAC_CONTAINER_NATIVE,
    MINIFLAC_CONTAINER_OGG,
};


struct miniflac_bitreader_s {
    uint64_t val;
    uint8_t  bits;
    uint8_t  crc8;
    uint16_t crc16;
    uint32_t pos;
    uint32_t len;
    const uint8_t* buffer;
};

struct miniflac_oggheader_s {
    MINIFLAC_OGGHEADER_STATE state;
};

struct miniflac_ogg_s {
    MINIFLAC_OGG_STATE state;
    miniflac_bitreader br; /* maintain our own bitreader */
    uint8_t version;
    uint8_t headertype;
    int64_t granulepos;
    int32_t serialno;
    uint32_t pageno;
    uint8_t segments;
    uint8_t curseg; /* current position within the segment table */
    uint16_t length; /* length of data within page */
    uint16_t pos; /* where we are within page */
};

struct miniflac_streammarker_s {
    MINIFLAC_STREAMMARKER_STATE state;
};

struct miniflac_metadata_header_s {
    MINIFLAC_METADATA_HEADER_STATE    state;
    uint8_t                        is_last;
    uint8_t                       type_raw;
    MINIFLAC_METADATA_TYPE             type;
    uint32_t                        length;
};

/* public-facing streaminfo struct */


enum MINIFLAC_STREAMINFO_STATE {
    MINIFLAC_STREAMINFO_MINBLOCKSIZE,
    MINIFLAC_STREAMINFO_MAXBLOCKSIZE,
    MINIFLAC_STREAMINFO_MINFRAMESIZE,
    MINIFLAC_STREAMINFO_MAXFRAMESIZE,
    MINIFLAC_STREAMINFO_SAMPLERATE,
    MINIFLAC_STREAMINFO_CHANNELS,
    MINIFLAC_STREAMINFO_BPS,
    MINIFLAC_STREAMINFO_TOTALSAMPLES,
    MINIFLAC_STREAMINFO_MD5_1,
    MINIFLAC_STREAMINFO_MD5_2,
    MINIFLAC_STREAMINFO_MD5_3,
    MINIFLAC_STREAMINFO_MD5_4,
};

struct miniflac_streaminfo_s {
    uint16_t        min_block_size;
    uint16_t        max_block_size;
    uint32_t        min_frame_size;
    uint32_t        max_frame_size;
    uint32_t           sample_rate;
    uint8_t               channels;
    uint8_t                    bps;
    uint64_t         total_samples;
    uint8_t                md5[16];
};

struct miniflac_streaminfo_private_s {
    miniflac_streaminfo_t      info;
    MINIFLAC_STREAMINFO_STATE state;
};

struct miniflac_vorbiscomment_s {
    MINIFLAC_VORBISCOMMENT_STATE    state;
    uint32_t len; /* length of the current string we're decoding */
    uint32_t pos; /* position within current string */
    uint32_t tot; /* total comments */
    uint32_t cur; /* current comment being decoded */
};

struct miniflac_metadata_s {
    MINIFLAC_METADATA_STATE          state;
    uint32_t                           pos;
    miniflac_metadata_header        header;
    miniflac_streaminfo_private_t  streaminfo;
    miniflac_vorbiscomment_t vorbiscomment;
};

struct miniflac_residual_s {
    MINIFLAC_RESIDUAL_STATE state;
    uint8_t coding_method;
    uint8_t partition_order;
    uint8_t rice_parameter;
    uint8_t rice_size;
    uint32_t msb; /* unsure what the max for this is */
    uint8_t rice_parameter_size; /* 4 or 5 based on coding method */
    int32_t value; /* current residual value */

    uint32_t partition; /* current partition */
    uint32_t partition_total; /* total partitions */

    uint32_t residual; /* current residual within partition */
    uint32_t residual_total; /* total residuals in partition */
};

struct miniflac_subframe_fixed_s {
    MINIFLAC_SUBFRAME_FIXED_STATE state;
    uint32_t pos;
    miniflac_residual residual;
};

struct miniflac_subframe_lpc_s {
    MINIFLAC_SUBFRAME_LPC_STATE state;
    uint32_t pos;
    uint8_t precision;
    uint8_t shift;
    uint8_t coeff;
    int32_t coefficients[32];
    miniflac_residual residual;
};

struct miniflac_subframe_constant_s {
    MINIFLAC_SUBFRAME_CONSTANT_STATE state;
};

struct miniflac_subframe_verbatim_s {
    MINIFLAC_SUBFRAME_VERBATIM_STATE state;
    uint32_t pos;
};

struct miniflac_subframe_header_s {
    MINIFLAC_SUBFRAME_HEADER_STATE state;
    MINIFLAC_SUBFRAME_TYPE type;
    uint8_t order;
    uint8_t wasted_bits;
    uint8_t type_raw;
};

struct miniflac_subframe_s {
    MINIFLAC_SUBFRAME_STATE state;
    uint8_t bps; /* effective bps for this subframe */
    miniflac_subframe_header header;
    miniflac_subframe_constant constant;
    miniflac_subframe_verbatim verbatim;
    miniflac_subframe_fixed fixed;
    miniflac_subframe_lpc lpc;
};

struct miniflac_frame_header_s {
    uint8_t block_size_raw; /* block size value direct from header */
    uint8_t sample_rate_raw; /* sample rate value direct from header */
    uint8_t channel_assignment_raw; /* channel assignment value direct from header */
    uint8_t  blocking_strategy;
    uint16_t block_size; /* calculated/parsed block size */
    uint32_t sample_rate; /* calculated/parsed sample rate */
    MINIFLAC_CHASSGN channel_assignment;
    uint8_t  channels;
    uint8_t  bps;
    union {
        uint64_t sample_number;
        uint32_t frame_number;
    };
    uint8_t crc8;
    MINIFLAC_FRAME_HEADER_STATE state;
};

struct miniflac_frame_s {
    MINIFLAC_FRAME_STATE state;
    uint8_t cur_subframe;
    uint16_t crc16;
    miniflac_frame_header header;
    miniflac_subframe subframe;
};

struct miniflac_s {
    MINIFLAC_STATE state;
    MINIFLAC_CONTAINER container;
    miniflac_bitreader br;
    miniflac_ogg_t ogg;
    miniflac_oggheader_t oggheader;
    miniflac_streammarker_t streammarker;
    miniflac_metadata metadata;
    miniflac_frame frame;
};


#ifdef __cplusplus
extern "C" {
#endif

/* returns the number of bytes needed for the miniflac struct (for malloc, etc) */
MINIFLAC_API
MINIFLAC_PURE
size_t
miniflac_size(void);

/* give the container type if you know the kind of container,
 * otherwise 0 for unknown */
MINIFLAC_API
void
miniflac_init(miniflac_t* pFlac, MINIFLAC_CONTAINER container);

/* sync to the next metadata block or frame, parses the metadata header or frame header */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_sync(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length);

/* decode a frame of audio, automatically skips metadata if needed */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_decode(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, int32_t** samples);

/* get the length of the vendor string, automatically skips metadata blocks, throws an error on audio frames */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_vendor_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* vendor_length);

/* get the vendor string, automatically skips metadata blocks, throws an error on audio frames */
/* will NOT be NULL-terminated! */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_vendor_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

/* get the total number of comments, automatically skips metadata blocks, throws an error on audio frames */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_comments_total(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* total_comments);

/* get the next comment length, automatically skips metadata blocks, throws an error on audio frames */
/* returns MINIFLAC_ITERATOR_END when out of comments */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_comment_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* comment_length);

/* get the next comment string, automatically skips metadata blocks, throws an error on audio frames */
/* will NOT be NULL-terminated! */
/* returns MINIFLAC_ITERATOR_END when out of comments */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_comment_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);


#ifdef __cplusplus
}
#endif

#endif

#ifdef MINIFLAC_IMPLEMENTATION

#ifdef MINIFLAC_ABORT_ON_ERROR
#include <stdlib.h>
#define miniflac_abort() abort()
#else
#define miniflac_abort()
#endif

MINIFLAC_PRIVATE
uint32_t
miniflac_unpack_uint32le(uint8_t buffer[4]);

MINIFLAC_PRIVATE
int32_t
miniflac_unpack_int32le(uint8_t buffer[4]);

MINIFLAC_PRIVATE
uint64_t
miniflac_unpack_uint64le(uint8_t buffer[8]);

MINIFLAC_PRIVATE
int64_t
miniflac_unpack_int64le(uint8_t buffer[8]);

MINIFLAC_PRIVATE
void
miniflac_bitreader_init(miniflac_bitreader* br);

MINIFLAC_PRIVATE
int
miniflac_bitreader_fill(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
int
miniflac_bitreader_fill_nocrc(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
uint64_t
miniflac_bitreader_read(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
int64_t
miniflac_bitreader_read_signed(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
uint64_t
miniflac_bitreader_peek(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
void
miniflac_bitreader_discard(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
void
miniflac_bitreader_align(miniflac_bitreader* br);

MINIFLAC_PRIVATE
void
miniflac_bitreader_reset_crc(miniflac_bitreader* br);

MINIFLAC_PRIVATE
void
miniflac_oggheader_init(miniflac_oggheader_t* oggheader);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_oggheader_decode(miniflac_oggheader_t* oggheader, miniflac_bitreader* br);

MINIFLAC_PRIVATE
void
miniflac_ogg_init(miniflac_ogg_t* ogg);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_ogg_sync(miniflac_ogg_t* ogg, miniflac_bitreader* br);

MINIFLAC_PRIVATE
void
miniflac_streammarker_init(miniflac_streammarker_t* streammarker);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streammarker_decode(miniflac_streammarker_t* streammarker, miniflac_bitreader* br);

MINIFLAC_PRIVATE
void
miniflac_metadata_header_init(miniflac_metadata_header* header);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_header_decode(miniflac_metadata_header* header, miniflac_bitreader *br);

MINIFLAC_PRIVATE
void
miniflac_streaminfo_init(miniflac_streaminfo_private_t* streaminfo);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_decode(miniflac_streaminfo_private_t* streaminfo, miniflac_bitreader* br);

MINIFLAC_PRIVATE
void
miniflac_vorbiscomment_init(miniflac_vorbiscomment_t* vorbiscomment);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_vendor_length(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_vendor_string(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, char* output, uint32_t length, uint32_t* outlen);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_total_comments(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, uint32_t* total);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_comment_length(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_comment_string(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, char* output, uint32_t length, uint32_t* outlen);

MINIFLAC_PRIVATE
void
miniflac_metadata_init(miniflac_metadata* metadata);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_sync(miniflac_metadata* metadata, miniflac_bitreader* br);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_decode(miniflac_metadata* metadata, miniflac_bitreader* br);

MINIFLAC_PRIVATE
void
miniflac_residual_init(miniflac_residual* residual);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_residual_decode(miniflac_residual* residual, miniflac_bitreader* br, uint32_t* pos, uint32_t block_size, uint8_t predictor_order, int32_t *out);

MINIFLAC_PRIVATE
void
miniflac_subframe_fixed_init(miniflac_subframe_fixed* c);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_fixed_decode(miniflac_subframe_fixed* c, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps, uint8_t predictor_order);

MINIFLAC_PRIVATE
void
miniflac_subframe_lpc_init(miniflac_subframe_lpc* l);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_lpc_decode(miniflac_subframe_lpc* l, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps, uint8_t predictor_order);

MINIFLAC_PRIVATE
void miniflac_subframe_constant_init(miniflac_subframe_constant* c);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_constant_decode(miniflac_subframe_constant* c, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps);

MINIFLAC_PRIVATE
void
miniflac_subframe_verbatim_init(miniflac_subframe_verbatim* c);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_verbatim_decode(miniflac_subframe_verbatim* c, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps);

MINIFLAC_PRIVATE
void
miniflac_subframe_header_init(miniflac_subframe_header* subframeheader);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_header_decode(miniflac_subframe_header* subframeheader, miniflac_bitreader* br);

MINIFLAC_PRIVATE
void
miniflac_subframe_init(miniflac_subframe* subframe);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_decode(miniflac_subframe* subframe, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps);

MINIFLAC_PRIVATE
void
miniflac_frame_header_init(miniflac_frame_header* frame_header);

MINIFLAC_PRIVATE
MINIFLAC_RESULT miniflac_frame_header_decode(miniflac_frame_header* frame_header, miniflac_bitreader* br);

MINIFLAC_PRIVATE
void miniflac_frame_init(miniflac_frame* frame);

/* ensures we've just read the audio frame header and are ready to decode */
MINIFLAC_PRIVATE
MINIFLAC_RESULT miniflac_frame_sync(miniflac_frame* frame, miniflac_bitreader* br, miniflac_streaminfo_private_t* info);

MINIFLAC_PRIVATE
MINIFLAC_RESULT miniflac_frame_decode(miniflac_frame* frame, miniflac_bitreader* br, miniflac_streaminfo_private_t* info, int32_t** output);



static
void
miniflac_oggreset(miniflac_t* pFlac) {
    miniflac_bitreader_init(&pFlac->br);
    miniflac_oggheader_init(&pFlac->oggheader);
    miniflac_streammarker_init(&pFlac->streammarker);
    miniflac_metadata_init(&pFlac->metadata);
    miniflac_frame_init(&pFlac->frame);
    pFlac->state = MINIFLAC_OGGHEADER;
}

static
MINIFLAC_RESULT
miniflac_oggfunction_start(miniflac_t* pFlac, const uint8_t* data, const uint8_t** packet, uint32_t* packet_length) {
    MINIFLAC_RESULT r;
    if(pFlac->ogg.state != MINIFLAC_OGG_DATA) {
        r = miniflac_ogg_sync(&pFlac->ogg,&pFlac->ogg.br);
        if(r != MINIFLAC_OK) return r;

        if(pFlac->ogg.headertype & 0x02) {
            miniflac_oggreset(pFlac);
        }
    }
    *packet = &data[pFlac->ogg.br.pos];
    *packet_length = pFlac->ogg.br.len - pFlac->ogg.br.pos;
    if(*packet_length > (uint32_t)(pFlac->ogg.length - pFlac->ogg.pos)) {
        *packet_length = (uint32_t)pFlac->ogg.length - pFlac->ogg.pos;
    }
    return MINIFLAC_OK;
}

static
void
miniflac_oggfunction_end(miniflac_t* pFlac, uint32_t packet_used) {
    pFlac->ogg.br.pos += packet_used;
    pFlac->ogg.pos    += packet_used;

    if(pFlac->ogg.pos == pFlac->ogg.length) {
        pFlac->ogg.state = MINIFLAC_OGG_CAPTUREPATTERN_O;
    }
}

MINIFLAC_API
MINIFLAC_PURE
size_t
miniflac_size(void) {
    return sizeof(miniflac_t);
}

MINIFLAC_API
void
miniflac_init(miniflac_t* pFlac, MINIFLAC_CONTAINER container) {
    miniflac_bitreader_init(&pFlac->br);
    miniflac_ogg_init(&pFlac->ogg);
    miniflac_oggheader_init(&pFlac->oggheader);
    miniflac_streammarker_init(&pFlac->streammarker);
    miniflac_metadata_init(&pFlac->metadata);
    miniflac_frame_init(&pFlac->frame);
    pFlac->container = container;

    switch(pFlac->container) {
        case MINIFLAC_CONTAINER_NATIVE: {
            pFlac->state = MINIFLAC_STREAMMARKER_OR_FRAME;
            break;
        }
        case MINIFLAC_CONTAINER_OGG: {
            pFlac->state = MINIFLAC_OGGHEADER;
            break;
        }
        default: break;
    }

    pFlac->state = MINIFLAC_STREAMMARKER;
}

static
MINIFLAC_RESULT
miniflac_sync_internal(miniflac_t* pFlac, miniflac_bitreader* br) {
    MINIFLAC_RESULT r;
    unsigned char c;
    uint16_t peek;

    switch(pFlac->state) {
        case MINIFLAC_OGGHEADER: {
            r = miniflac_oggheader_decode(&pFlac->oggheader,br);
            if (r != MINIFLAC_OK) return r;
            pFlac->state = MINIFLAC_STREAMMARKER;
            goto miniflac_sync_streammarker;
        }
        case MINIFLAC_STREAMMARKER_OR_FRAME: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            c = (unsigned char)miniflac_bitreader_peek(br,8);
            if( (char)c == 'f') {
                pFlac->state = MINIFLAC_STREAMMARKER;
                goto miniflac_sync_streammarker;
            } else if(c == 0xFF) {
                pFlac->state = MINIFLAC_FRAME;
                goto miniflac_sync_frame;
            }
            miniflac_abort();
            return MINIFLAC_ERROR;
        }

        case MINIFLAC_STREAMMARKER: {
            miniflac_sync_streammarker:
            r = miniflac_streammarker_decode(&pFlac->streammarker,br);
            if(r != MINIFLAC_OK) return r;
            pFlac->state = MINIFLAC_METADATA_OR_FRAME;
        }

        /* fall-through */
        case MINIFLAC_METADATA_OR_FRAME: {
            miniflac_sync_metadata_or_frame:
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            peek = (uint16_t)miniflac_bitreader_peek(br,14);
            if(peek == 0x3FFE) {
                pFlac->state = MINIFLAC_FRAME;
                goto miniflac_sync_frame;
            }
            pFlac->state = MINIFLAC_METADATA;
            goto miniflac_sync_metadata;
        }

        /* fall-through */
        case MINIFLAC_METADATA: {
            miniflac_sync_metadata:
            while(pFlac->metadata.state != MINIFLAC_METADATA_HEADER) {
                r = miniflac_metadata_decode(&pFlac->metadata,br);
                if(r != MINIFLAC_OK) return r;
                /* if we're here, it means we were in the middle of
                 * a metadata block and finished decoding, so the
                 * next block could be a metadata block or frame */
                pFlac->state = MINIFLAC_METADATA_OR_FRAME;
                goto miniflac_sync_metadata_or_frame;
            }
            return miniflac_metadata_sync(&pFlac->metadata,br);
        }

        case MINIFLAC_FRAME: {
            miniflac_sync_frame:
            while(pFlac->frame.state != MINIFLAC_FRAME_HEADER) {
                r = miniflac_frame_decode(&pFlac->frame,br,&pFlac->metadata.streaminfo,NULL);
                if(r != MINIFLAC_OK) return r;
            }

            return miniflac_frame_sync(&pFlac->frame,br,&pFlac->metadata.streaminfo);
        }
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}

static
MINIFLAC_RESULT
miniflac_sync_native(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    r = miniflac_sync_internal(pFlac,&pFlac->br);

    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_decode_native(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, int32_t** samples) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_FRAME) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_decode_exit;
    }

    r = miniflac_frame_decode(&pFlac->frame,&pFlac->br,&pFlac->metadata.streaminfo,samples);

    miniflac_decode_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_vendor_length_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* vendor_length) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_vendor_length_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_VORBIS_COMMENT) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_vendor_length_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_vendor_length_exit;
        }
    }

    r = miniflac_vorbiscomment_vendor_length(&pFlac->metadata.vorbiscomment,&pFlac->br,vendor_length);

    miniflac_vendor_length_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_vendor_string_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_vendor_string_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_VORBIS_COMMENT) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_vendor_string_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_vendor_string_exit;
        }
    }

    r = miniflac_vorbiscomment_vendor_string(&pFlac->metadata.vorbiscomment,&pFlac->br,buffer, buffer_length, buffer_used);

    miniflac_vendor_string_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_comments_total_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* total_comments) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_comments_total_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_VORBIS_COMMENT) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_comments_total_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_comments_total_exit;
        }
    }

    r = miniflac_vorbiscomment_total_comments(&pFlac->metadata.vorbiscomment,&pFlac->br,total_comments);

    miniflac_comments_total_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_comment_length_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* comment_length) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_comment_length_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_VORBIS_COMMENT) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_comment_length_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_comment_length_exit;
        }
    }

    r = miniflac_vorbiscomment_comment_length(&pFlac->metadata.vorbiscomment,&pFlac->br,comment_length);

    miniflac_comment_length_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_comment_string_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_comment_string_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_VORBIS_COMMENT) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_comment_string_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_comment_string_exit;
        }
    }

    r = miniflac_vorbiscomment_comment_string(&pFlac->metadata.vorbiscomment,&pFlac->br,buffer, buffer_length, buffer_used);

    miniflac_comment_string_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_sync_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length) {
    MINIFLAC_RESULT r = MINIFLAC_CONTINUE;

    const uint8_t* packet = NULL;
    uint32_t packet_length = 0;
    uint32_t packet_used   = 0;

    pFlac->ogg.br.buffer = data;
    pFlac->ogg.br.len = length;
    pFlac->ogg.br.pos = 0;

    do {
        r = miniflac_oggfunction_start(pFlac,data,&packet,&packet_length);
        if(r  != MINIFLAC_OK) break;

        r = miniflac_sync_native(pFlac,packet,packet_length,&packet_used);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_decode_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, int32_t** samples) {
    MINIFLAC_RESULT r = MINIFLAC_CONTINUE;

    const uint8_t* packet = NULL;
    uint32_t packet_length = 0;
    uint32_t packet_used   = 0;

    pFlac->ogg.br.buffer = data;
    pFlac->ogg.br.len = length;
    pFlac->ogg.br.pos = 0;

    do {
        r = miniflac_oggfunction_start(pFlac,data,&packet,&packet_length);
        if(r  != MINIFLAC_OK) break;

        r = miniflac_decode_native(pFlac,packet,packet_length,&packet_used,samples);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_vendor_length_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t *outlen) {
    MINIFLAC_RESULT r = MINIFLAC_CONTINUE;

    const uint8_t* packet = NULL;
    uint32_t packet_length = 0;
    uint32_t packet_used   = 0;

    pFlac->ogg.br.buffer = data;
    pFlac->ogg.br.len = length;
    pFlac->ogg.br.pos = 0;

    do {
        r = miniflac_oggfunction_start(pFlac,data,&packet,&packet_length);
        if(r  != MINIFLAC_OK) break;

        r = miniflac_vendor_length_native(pFlac,packet,packet_length,&packet_used,outlen);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_vendor_string_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r = MINIFLAC_CONTINUE;

    const uint8_t* packet = NULL;
    uint32_t packet_length = 0;
    uint32_t packet_used   = 0;

    pFlac->ogg.br.buffer = data;
    pFlac->ogg.br.len = length;
    pFlac->ogg.br.pos = 0;

    do {
        r = miniflac_oggfunction_start(pFlac,data,&packet,&packet_length);
        if(r  != MINIFLAC_OK) break;

        r = miniflac_vendor_string_native(pFlac,packet,packet_length,&packet_used,buffer,buffer_length,buffer_used);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_comments_total_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* total_comments) {
    MINIFLAC_RESULT r = MINIFLAC_CONTINUE;

    const uint8_t* packet = NULL;
    uint32_t packet_length = 0;
    uint32_t packet_used   = 0;

    pFlac->ogg.br.buffer = data;
    pFlac->ogg.br.len = length;
    pFlac->ogg.br.pos = 0;

    do {
        r = miniflac_oggfunction_start(pFlac,data,&packet,&packet_length);
        if(r  != MINIFLAC_OK) break;

        r = miniflac_comments_total_native(pFlac,packet,packet_length,&packet_used,total_comments);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_comment_length_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* comment_length) {
    MINIFLAC_RESULT r = MINIFLAC_CONTINUE;

    const uint8_t* packet = NULL;
    uint32_t packet_length = 0;
    uint32_t packet_used   = 0;

    pFlac->ogg.br.buffer = data;
    pFlac->ogg.br.len = length;
    pFlac->ogg.br.pos = 0;

    do {
        r = miniflac_oggfunction_start(pFlac,data,&packet,&packet_length);
        if(r  != MINIFLAC_OK) break;

        r = miniflac_comment_length_native(pFlac,packet,packet_length,&packet_used,comment_length);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_comment_string_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r = MINIFLAC_CONTINUE;

    const uint8_t* packet = NULL;
    uint32_t packet_length = 0;
    uint32_t packet_used   = 0;

    pFlac->ogg.br.buffer = data;
    pFlac->ogg.br.len = length;
    pFlac->ogg.br.pos = 0;

    do {
        r = miniflac_oggfunction_start(pFlac,data,&packet,&packet_length);
        if(r  != MINIFLAC_OK) break;

        r = miniflac_comment_string_native(pFlac,packet,packet_length,&packet_used,buffer,buffer_length,buffer_used);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_probe(miniflac_t* pFlac, const uint8_t* data, uint32_t length) {
    if(length == 0) return MINIFLAC_CONTINUE;
    switch(data[0]) {
        case 'f': {
            pFlac->container = MINIFLAC_CONTAINER_NATIVE;
            pFlac->state = MINIFLAC_STREAMMARKER;
            break;
        }
        case 'O': {
            pFlac->container = MINIFLAC_CONTAINER_OGG;
            pFlac->state = MINIFLAC_OGGHEADER;
            break;
        }
        default: return MINIFLAC_ERROR;
    }
    return MINIFLAC_OK;
}


MINIFLAC_API
MINIFLAC_RESULT
miniflac_decode(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, int32_t** samples) {
    MINIFLAC_RESULT r;

    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_decode_native(pFlac,data,length,out_length,samples);
    } else {
        r = miniflac_decode_ogg(pFlac,data,length,out_length,samples);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_sync(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length) {
    MINIFLAC_RESULT r;

    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_sync_native(pFlac,data,length,out_length);
    } else {
        r = miniflac_sync_ogg(pFlac,data,length,out_length);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_vendor_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* vendor_length) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_vendor_length_native(pFlac,data,length,out_length,vendor_length);
    } else {
        r = miniflac_vendor_length_ogg(pFlac,data,length,out_length,vendor_length);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_vendor_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_vendor_string_native(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    } else {
        r = miniflac_vendor_string_ogg(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_comments_total(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* comments_total) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_comments_total_native(pFlac,data,length,out_length,comments_total);
    } else {
        r = miniflac_comments_total_ogg(pFlac,data,length,out_length,comments_total);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_comment_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* comment_length) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_comment_length_native(pFlac,data,length,out_length,comment_length);
    } else {
        r = miniflac_comment_length_ogg(pFlac,data,length,out_length,comment_length);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_comment_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_comment_string_native(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    } else {
        r = miniflac_comment_string_ogg(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    }

    return r;
}
MINIFLAC_PRIVATE
uint32_t
miniflac_unpack_uint32le(uint8_t buffer[4]) {
    return (
      (((uint32_t)buffer[0]) << 0 ) |
      (((uint32_t)buffer[1]) << 8 ) |
      (((uint32_t)buffer[2]) << 16) |
      (((uint32_t)buffer[3]) << 24));
}

MINIFLAC_PRIVATE
int32_t
miniflac_unpack_int32le(uint8_t buffer[4]) {
    return (int32_t)miniflac_unpack_uint32le(buffer);
}

MINIFLAC_PRIVATE
uint64_t
miniflac_unpack_uint64le(uint8_t buffer[8]) {
    return (
      (((uint64_t)buffer[0]) << 0 ) |
      (((uint64_t)buffer[1]) << 8 ) |
      (((uint64_t)buffer[2]) << 16) |
      (((uint64_t)buffer[3]) << 24) |
      (((uint64_t)buffer[4]) << 32) |
      (((uint64_t)buffer[5]) << 40) |
      (((uint64_t)buffer[6]) << 48) |
      (((uint64_t)buffer[7]) << 56));
}

MINIFLAC_PRIVATE
int64_t
miniflac_unpack_int64le(uint8_t buffer[8]) {
    return (int64_t)miniflac_unpack_uint64le(buffer);
}

static const uint8_t miniflac_crc8_table[256] = {
  0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
  0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
  0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
  0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
  0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
  0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
  0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
  0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
  0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
  0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
  0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
  0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
  0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
  0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
  0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
  0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
  0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
  0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
  0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
  0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
  0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
  0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
  0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
  0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
  0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
  0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
  0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
  0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
  0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
  0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
  0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
  0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3,
};

static const uint16_t miniflac_crc16_table[256] = {
  0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
  0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
  0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
  0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
  0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
  0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
  0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
  0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,
  0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
  0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
  0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
  0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
  0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
  0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
  0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
  0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,
  0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
  0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
  0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
  0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
  0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
  0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
  0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
  0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,
  0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
  0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
  0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
  0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
  0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
  0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
  0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
  0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202,
};

MINIFLAC_PRIVATE
void
miniflac_bitreader_init(miniflac_bitreader* br) {
    br->val = 0;
    br->bits = 0;
    br->crc8 = 0;
    br->crc16 = 0;
    br->pos = 0;
    br->len = 0;
    br->buffer = NULL;
}

MINIFLAC_PRIVATE
int
miniflac_bitreader_fill(miniflac_bitreader* br, uint8_t bits) {
    uint8_t byte = 0;
    if(bits == 0) return 0;
    while(br->bits < bits && br->pos < br->len) {
        byte = br->buffer[br->pos++];
        br->val = (br->val << 8) | byte;
        br->bits += 8;
        br->crc8 = miniflac_crc8_table[br->crc8 ^ byte];
        br->crc16 = miniflac_crc16_table[ (br->crc16 >> 8) ^ byte ] ^ (( br->crc16 & 0x00FF ) << 8);
    }
    return br->bits < bits;
}

MINIFLAC_PRIVATE
int
miniflac_bitreader_fill_nocrc(miniflac_bitreader* br, uint8_t bits) {
    uint8_t byte = 0;
    if(bits == 0) return 0;
    while(br->bits < bits && br->pos < br->len) {
        byte = br->buffer[br->pos++];
        br->val = (br->val << 8) | byte;
        br->bits += 8;
    }
    return br->bits < bits;
}


MINIFLAC_PRIVATE
uint64_t
miniflac_bitreader_read(miniflac_bitreader* br, uint8_t bits) {
    uint64_t mask = -1LL;
    uint64_t imask = -1LL;
    uint64_t r;

    if(bits == 0) return 0;

    mask >>= (64 - bits);
    br->bits -= bits;
    r = br->val >> br->bits & mask;
    if(br->bits == 0) {
        imask = 0;
    } else {
        imask >>= (64 - br->bits);
    }
    br->val &= imask;

    return r;
}

MINIFLAC_PRIVATE
int64_t
miniflac_bitreader_read_signed(miniflac_bitreader* br, uint8_t bits) {
    uint64_t t;
    uint64_t mask = -1LL;
    if(bits == 0) return 0;
    mask <<= bits;

    t = miniflac_bitreader_read(br,bits);
    if( (t & ( 1 << (bits - 1))) ) {
        t |= mask;
    }
    return t;
}


MINIFLAC_PRIVATE
uint64_t
miniflac_bitreader_peek(miniflac_bitreader* br, uint8_t bits) {
    uint64_t mask = -1LL;
    uint64_t r;

    if(bits == 0) return 0;

    mask >>= (64 - bits);
    r = br->val >> (br->bits - bits) & mask;
    return r;
}

MINIFLAC_PRIVATE
void
miniflac_bitreader_discard(miniflac_bitreader* br, uint8_t bits) {
    uint64_t imask = -1LL;

    if(bits == 0) return;

    br->bits -= bits;

    if(br->bits == 0) {
        imask = 0;
    } else {
        imask >>= (64 - br->bits);
    }
    br->val &= imask;
}

MINIFLAC_PRIVATE
void
miniflac_bitreader_align(miniflac_bitreader* br) {
    br->bits = 0;
    br->val = 0;
}

MINIFLAC_PRIVATE
void
miniflac_bitreader_reset_crc(miniflac_bitreader* br) {
    uint64_t val = br->val;
    uint8_t bits = br->bits;

    uint8_t byte;
    uint64_t mask;
    uint64_t imask;

    br->crc8 = 0;
    br->crc16 = 0;

    while(bits > 0) {
        mask = -1LL;
        imask = -1LL;

        mask >>= (64 - 8);
        bits -= 8;
        byte = val >> bits & mask;
        if(bits == 0) {
            imask = 0;
        } else {
            imask >>= (64 - bits);
        }
        val &=  imask;
        br->crc8 = miniflac_crc8_table[br->crc8 ^ byte];
        br->crc16 = miniflac_crc16_table[ (br->crc16 >> 8) ^ byte ] ^ (( br->crc16 & 0x00FF ) << 8);
    }
}

MINIFLAC_PRIVATE
void
miniflac_oggheader_init(miniflac_oggheader_t* oggheader) {
    oggheader->state = MINIFLAC_OGGHEADER_PACKETTYPE;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_oggheader_decode(miniflac_oggheader_t* oggheader, miniflac_bitreader* br) {
    switch(oggheader->state) {
        case MINIFLAC_OGGHEADER_PACKETTYPE: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((uint8_t)miniflac_bitreader_read(br,8) != 0x7F) {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_F;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_F: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((char)miniflac_bitreader_read(br,8) != 'F') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_L;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_L: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((char)miniflac_bitreader_read(br,8) != 'L') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_A;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_A: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((char)miniflac_bitreader_read(br,8) != 'A') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_C;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_C: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((char)miniflac_bitreader_read(br,8) != 'C') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_MAJOR;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_MAJOR: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((uint8_t)miniflac_bitreader_read(br,8) != 0x01) {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_MINOR;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_MINOR: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((uint8_t)miniflac_bitreader_read(br,8) != 0x00) {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_HEADERPACKETS;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_HEADERPACKETS: {
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            miniflac_bitreader_discard(br,16);
            oggheader->state = MINIFLAC_OGGHEADER_PACKETTYPE;
        }
        /* fall-through */
        default: break;
    }
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
void
miniflac_ogg_init(miniflac_ogg_t* ogg) {
    ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_O;
    ogg->version = 0;
    ogg->headertype = 0;
    ogg->granulepos = 0;
    ogg->serialno = 0;
    ogg->pageno = 0;
    ogg->segments = 0;
    ogg->curseg = 0;
    ogg->length = 0;
    ogg->pos = 0;
    miniflac_bitreader_init(&ogg->br);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_ogg_sync(miniflac_ogg_t* ogg,miniflac_bitreader* br) {
    unsigned char c;
    uint8_t buffer[8];

    switch(ogg->state) {
        case MINIFLAC_OGG_DATA: {
            while(ogg->pos < ogg->length) {
                if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
                miniflac_bitreader_discard(br,8);
                ogg->pos++;
            }
            ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_O;
        }
        /* fall-through */
        case MINIFLAC_OGG_CAPTUREPATTERN_O: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            c = (unsigned char)miniflac_bitreader_read(br,8);
            if(c != 'O') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_G1;
        }
        /* fall-through */
        case MINIFLAC_OGG_CAPTUREPATTERN_G1: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            c = (unsigned char)miniflac_bitreader_read(br,8);
            if(c != 'g') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_G2;
        }
        /* fall-through */
        case MINIFLAC_OGG_CAPTUREPATTERN_G2: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            c = (unsigned char)miniflac_bitreader_read(br,8);
            if(c != 'g') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_S;
        }
        /* fall-through */
        case MINIFLAC_OGG_CAPTUREPATTERN_S: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            c = (unsigned char)miniflac_bitreader_read(br,8);
            if(c != 'S') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_VERSION;
        }
        /* fall-through */
        case MINIFLAC_OGG_VERSION: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            ogg->version = (uint8_t)miniflac_bitreader_read(br,8);
            if(ogg->version != 0) {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_HEADERTYPE;
        }
        /* fall-through */
        case MINIFLAC_OGG_HEADERTYPE: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            ogg->headertype = (uint8_t)miniflac_bitreader_read(br,8);
            ogg->state = MINIFLAC_OGG_GRANULEPOS;
        }
        /* fall-through */
        case MINIFLAC_OGG_GRANULEPOS: {
            if(miniflac_bitreader_fill_nocrc(br,64)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            buffer[4] = miniflac_bitreader_read(br,8);
            buffer[5] = miniflac_bitreader_read(br,8);
            buffer[6] = miniflac_bitreader_read(br,8);
            buffer[7] = miniflac_bitreader_read(br,8);
            ogg->granulepos = miniflac_unpack_int64le(buffer);
            ogg->state = MINIFLAC_OGG_SERIALNO;
        }
        /* fall-through */
        case MINIFLAC_OGG_SERIALNO: {
            if(miniflac_bitreader_fill_nocrc(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            ogg->serialno = miniflac_unpack_int32le(buffer);
            ogg->state = MINIFLAC_OGG_PAGENO;
        }
        /* fall-through */
        case MINIFLAC_OGG_PAGENO: {
            if(miniflac_bitreader_fill_nocrc(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            ogg->pageno = miniflac_unpack_uint32le(buffer);
            ogg->state = MINIFLAC_OGG_CHECKSUM;
        }
        /* fall-through */
        case MINIFLAC_OGG_CHECKSUM: {
            if(miniflac_bitreader_fill_nocrc(br,32)) return MINIFLAC_CONTINUE;
            miniflac_bitreader_discard(br,32);
            ogg->state = MINIFLAC_OGG_PAGESEGMENTS;
        }
        /* fall-through */
        case MINIFLAC_OGG_PAGESEGMENTS: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            ogg->segments = (uint8_t) miniflac_bitreader_read(br,8);
            ogg->curseg = 0;
            ogg->length = 0;
            ogg->state = MINIFLAC_OGG_SEGMENTTABLE;
        }
        /* fall-through */
        case MINIFLAC_OGG_SEGMENTTABLE: {
            while(ogg->curseg < ogg->segments) {
              if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
              ogg->length += miniflac_bitreader_read(br,8);
              ogg->curseg++;
            }
            ogg->pos = 0;
            ogg->state = MINIFLAC_OGG_DATA;
            return MINIFLAC_OK;
        }
    }
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
void
miniflac_frame_init(miniflac_frame* frame) {
    frame->crc16 = 0;
    frame->cur_subframe = 0;
    frame->state = MINIFLAC_FRAME_HEADER;
    miniflac_frame_header_init(&frame->header);
    miniflac_subframe_init(&frame->subframe);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_frame_sync(miniflac_frame* frame, miniflac_bitreader* br, miniflac_streaminfo_private_t* info) {
    MINIFLAC_RESULT r;
    assert(frame->state == MINIFLAC_FRAME_HEADER);
    r = miniflac_frame_header_decode(&frame->header,br);
    if(r != MINIFLAC_OK) return r;

    if(frame->header.sample_rate == 0) {
        if(info->info.sample_rate == 0) return MINIFLAC_FRAME_INVALID_SAMPLE_RATE;
        frame->header.sample_rate = info->info.sample_rate;
    }

    if(frame->header.bps == 0) {
        if(info->info.bps == 0) return MINIFLAC_FRAME_INVALID_SAMPLE_SIZE;
        frame->header.bps = info->info.bps;
    }

    frame->state = MINIFLAC_FRAME_SUBFRAME;
    frame->cur_subframe = 0;
    miniflac_subframe_init(&frame->subframe);
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_frame_decode(miniflac_frame* frame, miniflac_bitreader* br, miniflac_streaminfo_private_t* info, int32_t** output) {
    MINIFLAC_RESULT r;
    uint32_t bps;
    uint32_t i;
    uint64_t m,s;
    uint16_t t;
    switch(frame->state) {
        case MINIFLAC_FRAME_HEADER: {
            r = miniflac_frame_sync(frame,br,info);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_FRAME_SUBFRAME: {
            while(frame->cur_subframe < frame->header.channels) {
                bps = frame->header.bps;
                if(frame->header.channel_assignment == MINIFLAC_CHASSGN_LEFT_SIDE || frame->header.channel_assignment == MINIFLAC_CHASSGN_MID_SIDE) {
                    if(frame->cur_subframe == 1) bps += 1;
                } else if(frame->header.channel_assignment == MINIFLAC_CHASSGN_RIGHT_SIDE) {
                    if(frame->cur_subframe == 0) bps += 1;
                }
                r = miniflac_subframe_decode(&frame->subframe,br,output == NULL ? NULL : output[frame->cur_subframe],frame->header.block_size,bps);
                if(r != MINIFLAC_OK) return r;

                miniflac_subframe_init(&frame->subframe);
                frame->cur_subframe++;
            }

            miniflac_bitreader_align(br);
            frame->crc16 = br->crc16;
            frame->state = MINIFLAC_FRAME_FOOTER;
        }
        /* fall-through */
        case MINIFLAC_FRAME_FOOTER: {
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,16);
            if(frame->crc16 != t) {
                miniflac_abort();
                return MINIFLAC_FRAME_CRC16_INVALID;
            }
            if(output != NULL) {
                switch(frame->header.channel_assignment) {
                    case MINIFLAC_CHASSGN_LEFT_SIDE: {
                        for(i=0;i<frame->header.block_size;i++) {
                            output[1][i] = output[0][i] - output[1][i];
                        }
                        break;
                    }
                    case MINIFLAC_CHASSGN_RIGHT_SIDE: {
                        for(i=0;i<frame->header.block_size;i++) {
                            output[0][i] = output[0][i] + output[1][i];
                        }
                        break;
                    }
                    case MINIFLAC_CHASSGN_MID_SIDE: {
                        for(i=0;i<frame->header.block_size;i++) {
                            m = (uint64_t)output[0][i];
                            s = (uint64_t)output[1][i];
                            m = (m << 1) | (s & 0x01);
                            output[0][i] = (int32_t)((m + s) >> 1 );
                            output[1][i] = (int32_t)((m - s) >> 1 );
                        }
                        break;
                    }
                    default: break;
                }
            }
            break;
        }
        default: {
            /* invalid state */
            miniflac_abort();
            return MINIFLAC_ERROR;
        }
    }

    assert(br->bits == 0);
    br->crc8 = 0;
    br->crc16 = 0;
    frame->cur_subframe = 0;
    frame->state = MINIFLAC_FRAME_HEADER;
    miniflac_subframe_init(&frame->subframe);
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
void
miniflac_frame_header_init(miniflac_frame_header* header) {
    header->block_size_raw = 0;
    header->sample_rate_raw = 0;
    header->channel_assignment_raw = 0;
    header->sample_rate = 0;
    header->blocking_strategy = 0;
    header->block_size = 0;
    header->sample_rate = 0;
    header->channel_assignment = MINIFLAC_CHASSGN_NONE;
    header->channels = 0;
    header->bps = 0;
    header->sample_number = 0;
    header->crc8 = 0;
    header->state = MINIFLAC_FRAME_HEADER_SYNC;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_frame_header_decode(miniflac_frame_header* header, miniflac_bitreader* br) {
    uint64_t t;

    switch(header->state) {
        case MINIFLAC_FRAME_HEADER_SYNC: {
            if(miniflac_bitreader_fill(br,14)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,14);
            if(t != 0x3FFE) {
                miniflac_abort();
                return MINIFLAC_FRAME_SYNCCODE_INVALID;
            }
            miniflac_frame_header_init(header);
            header->state = MINIFLAC_FRAME_HEADER_RESERVEBIT_1;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_RESERVEBIT_1: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,1);
            if(t != 0) {
                miniflac_abort();
                return MINIFLAC_FRAME_RESERVED_BIT1;
            }
            header->state = MINIFLAC_FRAME_HEADER_BLOCKINGSTRATEGY;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_BLOCKINGSTRATEGY: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,1);
            header->blocking_strategy = t;
            header->state = MINIFLAC_FRAME_HEADER_BLOCKSIZE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_BLOCKSIZE: {
            if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,4);
            header->block_size_raw = t;
            header->block_size = 0;
            switch(header->block_size_raw) {
                case 0: {
                    miniflac_abort();
                    return MINIFLAC_FRAME_RESERVED_BLOCKSIZE;
                }
                case 1: {
                    header->block_size = 192;
                    break;
                }
                case 2: {
                    header->block_size = 576;
                    break;
                }
                case 3: {
                    header->block_size = 1152;
                    break;
                }
                case 4: {
                    header->block_size = 2304;
                    break;
                }
                case 5: {
                    header->block_size = 4608;
                    break;
                }
                case 8: {
                    header->block_size = 256;
                    break;
                }
                case 9: {
                    header->block_size = 512;
                    break;
                }
                case 10: {
                    header->block_size = 1024;
                    break;
                }
                case 11: {
                    header->block_size = 2048;
                    break;
                }
                case 12: {
                    header->block_size = 4096;
                    break;
                }
                case 13: {
                    header->block_size = 8192;
                    break;
                }
                case 14: {
                    header->block_size = 16384;
                    break;
                }
                case 15: {
                    header->block_size = 32768;
                    break;
                }
                default: break;
            }
            header->state = MINIFLAC_FRAME_HEADER_SAMPLERATE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLERATE: {
            if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,4);
            header->sample_rate_raw = t;
            switch(header->sample_rate_raw) {
                case 0: {
                    header->sample_rate = 0;
                    break;
                }
                case 1: {
                    header->sample_rate = 88200;
                    break;
                }
                case 2: {
                    header->sample_rate = 176400;
                    break;
                }
                case 3: {
                    header->sample_rate = 192000;
                    break;
                }
                case 4: {
                    header->sample_rate = 8000;
                    break;
                }
                case 5: {
                    header->sample_rate = 16000;
                    break;
                }
                case 6: {
                    header->sample_rate = 22050;
                    break;
                }
                case 7: {
                    header->sample_rate = 24000;
                    break;
                }
                case 8: {
                    header->sample_rate = 32000;
                    break;
                }
                case 9: {
                    header->sample_rate = 44100;
                    break;
                }
                case 10: {
                    header->sample_rate = 48000;
                    break;
                }
                case 11: {
                    header->sample_rate = 96000;
                    break;
                }
                case 12: /* fall-through */
                case 13: /* fall-through */
                case 14: {
                    header->sample_rate = 0;
                    break;
                }
                case 15: {
                    miniflac_abort();
                    return MINIFLAC_FRAME_INVALID_SAMPLE_RATE;
                }
                default: break;
            }

            header->state = MINIFLAC_FRAME_HEADER_CHANNELASSIGNMENT;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_CHANNELASSIGNMENT: {
            if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,4);
            if(t > 10) {
                miniflac_abort();
                return MINIFLAC_FRAME_RESERVED_CHANNEL_ASSIGNMENT;
            }

            if(t < 8) {
                header->channels = t + 1;
                header->channel_assignment = MINIFLAC_CHASSGN_NONE;
            } else {
                switch(t) {
                    case 8: {
                        header->channel_assignment = MINIFLAC_CHASSGN_LEFT_SIDE; break;
                    }
                    case 9: {
                        header->channel_assignment = MINIFLAC_CHASSGN_RIGHT_SIDE; break;
                    }
                    case 10: {
                        header->channel_assignment = MINIFLAC_CHASSGN_MID_SIDE; break;
                    }
                    default: break;
                }
                header->channels = 2;
            }

            header->channel_assignment_raw = t;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLESIZE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLESIZE: {
            if(miniflac_bitreader_fill(br,3)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,3);
            switch(t) {
                case 0: header->bps = 0; break;
                case 1: header->bps = 8; break;
                case 2: header->bps = 12; break;
                case 3: {
                    miniflac_abort();
                    return MINIFLAC_FRAME_RESERVED_SAMPLE_SIZE;
                }
                case 4: header->bps = 16; break;
                case 5: header->bps = 20; break;
                case 6: header->bps = 24; break;
                case 7: {
                    miniflac_abort();
                    return MINIFLAC_FRAME_RESERVED_SAMPLE_SIZE;
                }
            }
            header->state = MINIFLAC_FRAME_HEADER_RESERVEBIT_2;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_RESERVEBIT_2: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,1);
            if(t != 0) {
                miniflac_abort();
                return MINIFLAC_FRAME_RESERVED_BIT2;
            }
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_1;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_1: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);

            if((t & 0x80) == 0x00) {
                header->sample_number = t;
                header->state = MINIFLAC_FRAME_HEADER_BLOCKSIZE_MAYBE;
                goto flac_frame_blocksize_maybe;
            }
            else if((t & 0xE0) == 0xC0) {
                header->sample_number = (t & 0x1F) << 6;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_7;
                goto flac_frame_samplenumber_7;
            } else if((t & 0xF0) == 0xE0) {
                header->sample_number = (t & 0x0F) << 12;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_6;
                goto flac_frame_samplenumber_6;
            } else if((t & 0xF8) == 0xF0) {
                header->sample_number = (t & 0x07) << 18;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_5;
                goto flac_frame_samplenumber_5;
            } else if((t & 0xFC) == 0xF8) {
                header->sample_number = (t & 0x03) << 24;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_4;
                goto flac_frame_samplenumber_4;
            } else if((t & 0xFF) == 0xFE) {
                header->sample_number = (t & 0x01) << 30;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_3;
                goto flac_frame_samplenumber_3;
            } else if((t & 0xFF) == 0xFF) {
                header->sample_number = 0;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_2;
                goto flac_frame_samplenumber_2;
            }
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_2: {
            flac_frame_samplenumber_2:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 30;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_3;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_3: {
            flac_frame_samplenumber_3:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 24;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_4;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_4: {
            flac_frame_samplenumber_4:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 18;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_5;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_5: {
            flac_frame_samplenumber_5:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 12;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_6;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_6: {
            flac_frame_samplenumber_6:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 6;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_7;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_7: {
            flac_frame_samplenumber_7:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F);
            header->state = MINIFLAC_FRAME_HEADER_BLOCKSIZE_MAYBE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_BLOCKSIZE_MAYBE: {
            flac_frame_blocksize_maybe:
            switch(header->block_size_raw) {
                case 6: {
                    if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,8) + 1;
                    header->block_size = t;
                    break;
                }
                case 7: {
                    if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,16) + 1;
                    header->block_size = t;
                    break;
                }
                default: break;
            }
            header->state = MINIFLAC_FRAME_HEADER_SAMPLERATE_MAYBE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLERATE_MAYBE: {
            switch(header->sample_rate_raw) {
                case 12: {
                    if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,8);
                    header->sample_rate = t * 1000;
                    break;
                }
                case 13: {
                    if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,16);
                    header->sample_rate = t;
                    break;
                }
                case 14: {
                    if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,16);
                    header->sample_rate = t * 10;
                    break;
                }
                default: break;
            }

            /* grab crc8 from bitreader before fill */
            header->crc8 = br->crc8;
            header->state = MINIFLAC_FRAME_HEADER_CRC8;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_CRC8: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            if(header->crc8 != t) {
                miniflac_abort();
                return MINIFLAC_FRAME_CRC8_INVALID;
            }
        }
        /* fall-through */
        default: break;
    }

    header->state = MINIFLAC_FRAME_HEADER_SYNC;
    return MINIFLAC_OK;
}


MINIFLAC_PRIVATE
void
miniflac_vorbiscomment_init(miniflac_vorbiscomment_t* vorbiscomment) {
    vorbiscomment->state = MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH;
    vorbiscomment->len = 0;
    vorbiscomment->pos = 0;
    vorbiscomment->tot = 0;
    vorbiscomment->cur = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_vendor_length(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, uint32_t* length) {
    uint8_t buffer[4];
    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            vorbiscomment->len = miniflac_unpack_uint32le(buffer);
            if(length != NULL) *length = vorbiscomment->len;
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_VENDOR_STRING;
            return MINIFLAC_OK;
        } default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_vendor_string(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, char* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    char c;

    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: {
            r = miniflac_vorbiscomment_vendor_length(vorbiscomment,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING: {
            while(vorbiscomment->pos < vorbiscomment->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && vorbiscomment->pos < length) {
                    output[vorbiscomment->pos] = c;
                }
                vorbiscomment->pos++;
            }
            if(outlen != NULL) {
                *outlen = vorbiscomment->len <= length ? vorbiscomment->len : length;
            }
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_total_comments(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, uint32_t* total) {
    uint8_t buffer[4];
    MINIFLAC_RESULT r = MINIFLAC_ERROR;

    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: /* fall-through */
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING: {
            r = miniflac_vorbiscomment_vendor_string(vorbiscomment,br,NULL,0,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            vorbiscomment->tot = miniflac_unpack_uint32le(buffer);
            if(total != NULL) *total = vorbiscomment->tot;
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_comment_length(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, uint32_t* length) {
    uint8_t buffer[4];
    MINIFLAC_RESULT r = MINIFLAC_ERROR;

    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: /* fall-through */
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING: /* fall-through */
        case MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS: {
            r = miniflac_vorbiscomment_total_comments(vorbiscomment,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH: {
            case_miniflac_vorbiscomment_comment_length:
            if(vorbiscomment->cur == vorbiscomment->tot) {
                return MINIFLAC_ITERATOR_END;
            }

            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            vorbiscomment->len = miniflac_unpack_uint32le(buffer);
            vorbiscomment->pos = 0;
            if(length != NULL) *length = vorbiscomment->len;
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_COMMENT_STRING;
            return MINIFLAC_OK;
        }
        case MINIFLAC_VORBISCOMMENT_COMMENT_STRING: {
            r = miniflac_vorbiscomment_comment_string(vorbiscomment,br,NULL,0,NULL);
            if(r != MINIFLAC_OK) return r;
            goto case_miniflac_vorbiscomment_comment_length;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_comment_string(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, char* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    char c;

    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH:
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING:
        case MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS:
        case MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH: {
            r = miniflac_vorbiscomment_comment_length(vorbiscomment,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_COMMENT_STRING: {
            while(vorbiscomment->pos < vorbiscomment->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && vorbiscomment->pos < length) {
                    output[vorbiscomment->pos] = c;
                }
                vorbiscomment->pos++;
            }
            if(outlen != NULL) {
                *outlen = vorbiscomment->len <= length ? vorbiscomment->len : length;
            }
            vorbiscomment->cur++;
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
void
miniflac_metadata_init(miniflac_metadata* metadata) {
    metadata->state = MINIFLAC_METADATA_HEADER;
    metadata->pos = 0;
    miniflac_metadata_header_init(&metadata->header);
    miniflac_streaminfo_init(&metadata->streaminfo);
    miniflac_vorbiscomment_init(&metadata->vorbiscomment);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_sync(miniflac_metadata* metadata, miniflac_bitreader* br) {
    MINIFLAC_RESULT r;
    assert(metadata->state == MINIFLAC_METADATA_HEADER);
    r = miniflac_metadata_header_decode(&metadata->header,br);
    if(r != MINIFLAC_OK) return r;

    metadata->state = MINIFLAC_METADATA_DATA;
    metadata->pos = 0;
    return MINIFLAC_OK;
}

static
MINIFLAC_RESULT
miniflac_metadata_skip(miniflac_metadata* metadata, miniflac_bitreader *br) {
    while(metadata->pos < metadata->header.length) {
        if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
        miniflac_bitreader_discard(br,8);
        metadata->pos++;
    }
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_decode(miniflac_metadata* metadata, miniflac_bitreader *br) {
    MINIFLAC_RESULT r;
    switch(metadata->state) {
        case MINIFLAC_METADATA_HEADER: {
            r = miniflac_metadata_sync(metadata,br);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_METADATA_DATA: {
            switch(metadata->header.type) {
                case MINIFLAC_METADATA_STREAMINFO: {
                    r = miniflac_streaminfo_decode(&metadata->streaminfo,br);
                    break;
                }
                case MINIFLAC_METADATA_VORBIS_COMMENT: {
                    do {
                        r = miniflac_vorbiscomment_comment_length(&metadata->vorbiscomment,br,NULL);
                    } while(r == MINIFLAC_OK);
                    if(r == MINIFLAC_ITERATOR_END) {
                        r = MINIFLAC_OK;
                    }
                    break;
                }
                default: {
                    r = miniflac_metadata_skip(metadata,br);
                }
            }
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        default: break;
    }

    assert(br->bits == 0);
    br->crc8  = 0;
    br->crc16 = 0;
    metadata->state = MINIFLAC_METADATA_HEADER;
    metadata->pos   = 0;
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
void
miniflac_metadata_header_init(miniflac_metadata_header* header) {
    header->state = MINIFLAC_METADATA_LAST_FLAG;
    header->type = MINIFLAC_METADATA_UNKNOWN;
    header->is_last = 0;
    header->type_raw = 0;
    header->length = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_header_decode(miniflac_metadata_header* header, miniflac_bitreader *br) {
    switch(header->state) {
        case MINIFLAC_METADATA_LAST_FLAG: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            miniflac_metadata_header_init(header);
            header->is_last = (uint8_t)miniflac_bitreader_read(br,1);
            header->state = MINIFLAC_METADATA_BLOCK_TYPE;
        }
        /* fall-through */
        case MINIFLAC_METADATA_BLOCK_TYPE: {
            if(miniflac_bitreader_fill(br,7)) return MINIFLAC_CONTINUE;
            header->type_raw = (uint8_t)miniflac_bitreader_read(br,7);
            switch(header->type_raw) {
                case 0: header->type = MINIFLAC_METADATA_STREAMINFO; break;
                case 1: header->type = MINIFLAC_METADATA_PADDING; break;
                case 2: header->type = MINIFLAC_METADATA_APPLICATION; break;
                case 3: header->type = MINIFLAC_METADATA_SEEKTABLE; break;
                case 4: header->type = MINIFLAC_METADATA_VORBIS_COMMENT; break;
                case 5: header->type = MINIFLAC_METADATA_CUESHEET; break;
                case 6: header->type = MINIFLAC_METADATA_PICTURE; break;
                case 127: {
                    header->type = MINIFLAC_METADATA_INVALID;
                    miniflac_abort();
                    return MINIFLAC_METADATA_TYPE_INVALID;
                }
                default: {
                    header->type = MINIFLAC_METADATA_UNKNOWN;
                    miniflac_abort();
                    return MINIFLAC_METADATA_TYPE_RESERVED;
                }
            }
            header->state = MINIFLAC_METADATA_LENGTH;
        }
        /* fall-through */
        case MINIFLAC_METADATA_LENGTH: {
            if(miniflac_bitreader_fill(br,24)) return MINIFLAC_CONTINUE;
            header->length = (uint32_t) miniflac_bitreader_read(br,24);
            header->state = MINIFLAC_METADATA_LAST_FLAG;
            break;
        }
        default: break;
    }
    return MINIFLAC_OK;
}

static const uint8_t escape_codes[2] = {
    15,
    31,
};

MINIFLAC_PRIVATE
void
miniflac_residual_init(miniflac_residual* residual) {
    residual->coding_method = 0;
    residual->partition_order = 0;
    residual->rice_parameter = 0;
    residual->rice_size = 0;
    residual->msb = 0;
    residual->rice_parameter_size = 0;
    residual->value = 0;
    residual->partition = 0;
    residual->partition_total = 0;
    residual->residual = 0;
    residual->residual_total = 0;
    residual->state = MINIFLAC_RESIDUAL_CODING_METHOD;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_residual_decode(miniflac_residual* residual, miniflac_bitreader* br, uint32_t* pos, uint32_t block_size, uint8_t predictor_order, int32_t *output) {
    uint64_t temp;
    uint32_t temp_32;

    switch(residual->state) {
        case MINIFLAC_RESIDUAL_CODING_METHOD: {
            if(miniflac_bitreader_fill(br,2)) return MINIFLAC_CONTINUE;
            temp = miniflac_bitreader_read(br,2);
            if(temp > 1) {
                miniflac_abort();
                return MINIFLAC_RESERVED_CODING_METHOD;
            }
            residual->coding_method = temp;
            switch(residual->coding_method) {
                case 0: {
                    residual->rice_parameter_size = 4;
                    break;
                }
                case 1: {
                    residual->rice_parameter_size = 5;
                    break;
                }
            }
            residual->msb = 0;
            residual->state = MINIFLAC_RESIDUAL_PARTITION_ORDER;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_PARTITION_ORDER: {
            if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
            residual->partition_order = miniflac_bitreader_read(br,4);
            residual->partition_total = 1 << residual->partition_order;
            residual->state = MINIFLAC_RESIDUAL_RICE_PARAMETER;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_RICE_PARAMETER: {
            miniflac_residual_rice_parameter:
            if(miniflac_bitreader_fill(br,residual->rice_parameter_size)) return MINIFLAC_CONTINUE;
            residual->rice_parameter = miniflac_bitreader_read(br,residual->rice_parameter_size);

            residual->residual = 0;
            residual->residual_total = block_size >> residual->partition_order;
            if(residual->partition == 0) {
                residual->residual_total -= predictor_order;
            }

            if(residual->rice_parameter == escape_codes[residual->coding_method]) {
                residual->state = MINIFLAC_RESIDUAL_RICE_SIZE;
                goto miniflac_residual_rice_size;
            }
            residual->state = MINIFLAC_RESIDUAL_MSB;
            goto miniflac_residual_msb;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_RICE_SIZE: {
            miniflac_residual_rice_size:
            if(miniflac_bitreader_fill(br,5)) return MINIFLAC_CONTINUE;
            residual->rice_size = miniflac_bitreader_read(br,5);
            residual->state = MINIFLAC_RESIDUAL_RICE_VALUE;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_RICE_VALUE: {
            miniflac_residual_rice_value:
            if(miniflac_bitreader_fill(br,residual->rice_size)) return MINIFLAC_CONTINUE;
            residual->value = miniflac_bitreader_read_signed(br,residual->rice_size);
            if(output != NULL) {
                output[*pos] = residual->value;
            }
            *pos += 1;
            residual->residual++;
            if(residual->residual < residual->residual_total) {
                residual->state = MINIFLAC_RESIDUAL_RICE_VALUE;
                goto miniflac_residual_rice_value;
            }
            goto miniflac_residual_nextpart;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_MSB: {
            miniflac_residual_msb:
            while(!miniflac_bitreader_fill(br,1)) {
                if(miniflac_bitreader_read(br,1)) {
                    residual->state = MINIFLAC_RESIDUAL_LSB;
                    goto miniflac_residual_lsb;
                }
                residual->msb++;
            }
            return MINIFLAC_CONTINUE;
        }
        case MINIFLAC_RESIDUAL_LSB: {
            miniflac_residual_lsb:
            if(miniflac_bitreader_fill(br,residual->rice_parameter)) return MINIFLAC_CONTINUE;
            temp_32 = (residual->msb << residual->rice_parameter) | ((uint32_t)miniflac_bitreader_read(br,residual->rice_parameter));
            residual->value = (temp_32 >> 1) ^ -(temp_32 & 1);

            if(output != NULL) {
                output[*pos] = residual->value;
            }
            *pos += 1;

            residual->msb = 0;
            residual->residual++;
            if(residual->residual < residual->residual_total) {
                residual->state = MINIFLAC_RESIDUAL_MSB;
                goto miniflac_residual_msb;
            }

            miniflac_residual_nextpart:
            residual->residual = 0;
            residual->partition++;
            if(residual->partition < residual->partition_total) {
                residual->state = MINIFLAC_RESIDUAL_RICE_PARAMETER;
                goto miniflac_residual_rice_parameter;
            }
            break;
        }
        default: break;
    }

    miniflac_residual_init(residual);
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
void
miniflac_streaminfo_init(miniflac_streaminfo_private_t* streaminfo) {
    unsigned int i;
    streaminfo->state = MINIFLAC_STREAMINFO_MINBLOCKSIZE;
    streaminfo->info.min_block_size = 0;
    streaminfo->info.max_block_size = 0;
    streaminfo->info.min_frame_size = 0;
    streaminfo->info.max_frame_size = 0;
    streaminfo->info.sample_rate = 0;
    streaminfo->info.channels = 0;
    streaminfo->info.bps = 0;
    streaminfo->info.total_samples = 0;
    for(i=0;i<16;i++) {
        streaminfo->info.md5[i] = 0;
    }
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_decode(miniflac_streaminfo_private_t* streaminfo, miniflac_bitreader* br) {
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: {
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            streaminfo->info.min_block_size = (uint16_t) miniflac_bitreader_read(br,16);
            streaminfo->state = MINIFLAC_STREAMINFO_MAXBLOCKSIZE;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: {
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            streaminfo->info.max_block_size = (uint16_t) miniflac_bitreader_read(br,16);
            streaminfo->state = MINIFLAC_STREAMINFO_MINFRAMESIZE;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: {
            if(miniflac_bitreader_fill(br,24)) return MINIFLAC_CONTINUE;
            streaminfo->info.min_frame_size = (uint32_t) miniflac_bitreader_read(br,24);
            streaminfo->state = MINIFLAC_STREAMINFO_MAXFRAMESIZE;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MAXFRAMESIZE: {
            if(miniflac_bitreader_fill(br,24)) return MINIFLAC_CONTINUE;
            streaminfo->info.max_frame_size = (uint32_t) miniflac_bitreader_read(br,24);
            streaminfo->state = MINIFLAC_STREAMINFO_SAMPLERATE;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_SAMPLERATE: {
            if(miniflac_bitreader_fill(br,20)) return MINIFLAC_CONTINUE;
            streaminfo->info.sample_rate = (uint32_t) miniflac_bitreader_read(br,20);
            streaminfo->state = MINIFLAC_STREAMINFO_CHANNELS;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_CHANNELS: {
            if(miniflac_bitreader_fill(br,3)) return MINIFLAC_CONTINUE;
            streaminfo->info.channels = (uint8_t) miniflac_bitreader_read(br,3) + 1;
            streaminfo->state = MINIFLAC_STREAMINFO_BPS;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_BPS: {
            if(miniflac_bitreader_fill(br,5)) return MINIFLAC_CONTINUE;
            streaminfo->info.bps = (uint8_t) miniflac_bitreader_read(br,5) + 1;
            streaminfo->state = MINIFLAC_STREAMINFO_TOTALSAMPLES;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_TOTALSAMPLES: {
            if(miniflac_bitreader_fill(br,36)) return MINIFLAC_CONTINUE;
            streaminfo->info.total_samples = (uint64_t) miniflac_bitreader_read(br,36);
            streaminfo->state = MINIFLAC_STREAMINFO_MD5_1;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MD5_1: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            streaminfo->info.md5[0] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[1] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[2] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[3] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->state = MINIFLAC_STREAMINFO_MD5_2;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MD5_2: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            streaminfo->info.md5[4] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[5] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[6] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[7] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->state = MINIFLAC_STREAMINFO_MD5_3;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MD5_3: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            streaminfo->info.md5[8] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[9] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[10] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[11] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->state = MINIFLAC_STREAMINFO_MD5_4;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MD5_4: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            streaminfo->info.md5[12] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[13] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[14] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[15] = (uint8_t) miniflac_bitreader_read(br,8);
            break;
        }
        default: {
            miniflac_abort();
            return MINIFLAC_ERROR;
        }
    }

    streaminfo->state = MINIFLAC_STREAMINFO_MINBLOCKSIZE;
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
void
miniflac_streammarker_init(miniflac_streammarker_t* streammarker) {
    streammarker->state = MINIFLAC_STREAMMARKER_F;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streammarker_decode(miniflac_streammarker_t* streammarker, miniflac_bitreader* br) {
    char t;
    switch(streammarker->state) {
        case MINIFLAC_STREAMMARKER_F: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = (char)miniflac_bitreader_read(br,8);
            if(t != 'f') {
                miniflac_abort();
                return MINIFLAC_STREAMMARKER_INVALID;
            }
            streammarker->state = MINIFLAC_STREAMMARKER_L;
        }
        /* fall-through */
        case MINIFLAC_STREAMMARKER_L: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = (char)miniflac_bitreader_read(br,8);
            if(t != 'L') {
                miniflac_abort();
                return MINIFLAC_STREAMMARKER_INVALID;
            }
            streammarker->state = MINIFLAC_STREAMMARKER_A;
        }
        /* fall-through */
        case MINIFLAC_STREAMMARKER_A: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = (char)miniflac_bitreader_read(br,8);
            if(t != 'a') {
                miniflac_abort();
                return MINIFLAC_STREAMMARKER_INVALID;
            }
            streammarker->state = MINIFLAC_STREAMMARKER_C;
        }
        /* fall-through */
        case MINIFLAC_STREAMMARKER_C: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = (char)miniflac_bitreader_read(br,8);
            if(t != 'C') {
                miniflac_abort();
                return MINIFLAC_STREAMMARKER_INVALID;
            }
            break;
        }
        default: {
            miniflac_abort();
            return MINIFLAC_ERROR;
        }
    }

    miniflac_streammarker_init(streammarker);
    miniflac_bitreader_reset_crc(br);
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
void
miniflac_subframe_init(miniflac_subframe* subframe) {
    subframe->bps = 0;
    subframe->state = MINIFLAC_SUBFRAME_HEADER;
    miniflac_subframe_header_init(&subframe->header);
    miniflac_subframe_constant_init(&subframe->constant);
    miniflac_subframe_verbatim_init(&subframe->verbatim);
    miniflac_subframe_fixed_init(&subframe->fixed);
    miniflac_subframe_lpc_init(&subframe->lpc);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_decode(miniflac_subframe* subframe, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps) {
    MINIFLAC_RESULT r;
    uint32_t i;

    switch(subframe->state) {
        case MINIFLAC_SUBFRAME_HEADER: {
            r = miniflac_subframe_header_decode(&subframe->header,br);
            if(r != MINIFLAC_OK) return r;

            subframe->bps = bps - subframe->header.wasted_bits;

            switch(subframe->header.type) {
                case MINIFLAC_SUBFRAME_TYPE_CONSTANT: {
                    subframe->state = MINIFLAC_SUBFRAME_CONSTANT;
                    goto miniflac_subframe_constant;
                }
                case MINIFLAC_SUBFRAME_TYPE_VERBATIM: {
                    subframe->state = MINIFLAC_SUBFRAME_VERBATIM;
                    goto miniflac_subframe_verbatim;
                }
                case MINIFLAC_SUBFRAME_TYPE_FIXED: {
                    subframe->state = MINIFLAC_SUBFRAME_FIXED;
                    goto miniflac_subframe_fixed;
                }
                case MINIFLAC_SUBFRAME_TYPE_LPC: {
                    subframe->state = MINIFLAC_SUBFRAME_LPC;
                    goto miniflac_subframe_lpc;
                }
                default: {
                    miniflac_abort();
                    return MINIFLAC_ERROR;
                }
            }
            break;
        }

        case MINIFLAC_SUBFRAME_CONSTANT: {
            miniflac_subframe_constant:
            r = miniflac_subframe_constant_decode(&subframe->constant,br,output,block_size,subframe->bps);
            if(r != MINIFLAC_OK) return r;
            break;
        }
        case MINIFLAC_SUBFRAME_VERBATIM: {
            miniflac_subframe_verbatim:
            r = miniflac_subframe_verbatim_decode(&subframe->verbatim,br,output,block_size,subframe->bps);
            if(r != MINIFLAC_OK) return r;
            break;
        }
        case MINIFLAC_SUBFRAME_FIXED: {
            miniflac_subframe_fixed:
            r = miniflac_subframe_fixed_decode(&subframe->fixed,br,output,block_size,subframe->bps,subframe->header.order);
            if(r != MINIFLAC_OK) return r;
            break;
        }
        case MINIFLAC_SUBFRAME_LPC: {
            miniflac_subframe_lpc:
            r = miniflac_subframe_lpc_decode(&subframe->lpc,br,output,block_size,subframe->bps,subframe->header.order);
            if(r != MINIFLAC_OK) return r;
            break;
        }
        default: break;
    }

    if(subframe->header.wasted_bits) {
        for(i=0;i<block_size;i++) {
            output[i] <<= subframe->header.wasted_bits;
        }
    }

    miniflac_subframe_init(subframe);
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
void
miniflac_subframe_constant_init(miniflac_subframe_constant *c) {
    c->state = MINIFLAC_SUBFRAME_CONSTANT_DECODE;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_constant_decode(miniflac_subframe_constant* c, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps) {
    int32_t sample;
    uint32_t i;
    (void)c;

    if(miniflac_bitreader_fill(br,bps)) return MINIFLAC_CONTINUE;
    sample = (int32_t) miniflac_bitreader_read_signed(br,bps);

    if(output != NULL) {
        for(i=0;i<block_size;i++) {
            output[i] = sample;
        }
    }

    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
void
miniflac_subframe_fixed_init(miniflac_subframe_fixed *f) {
    f->pos   = 0;
    f->state = MINIFLAC_SUBFRAME_FIXED_DECODE;
    miniflac_residual_init(&f->residual);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_fixed_decode(miniflac_subframe_fixed* f, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps, uint8_t predictor_order) {
    int32_t sample;

    int64_t sample1;
    int64_t sample2;
    int64_t sample3;
    int64_t sample4;
    int64_t residual;

    MINIFLAC_RESULT r;
    while(f->pos < predictor_order) {
        if(miniflac_bitreader_fill(br,bps)) return MINIFLAC_CONTINUE;
        sample = (int32_t) miniflac_bitreader_read_signed(br,bps);
        if(output != NULL) {
            output[f->pos] = sample;
        }
        f->pos++;
    }
    r = miniflac_residual_decode(&f->residual,br,&f->pos,block_size,predictor_order,output);
    if(r != MINIFLAC_OK) return r;

    if(output != NULL) {
        switch(predictor_order) {
            case 0:
#if 0
                /* this is here for reference but not actually needed */
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    residual = output[f->pos];
                    output[f->pos] = (int32_t)residual;
                }
#endif
                break;
            case 1: {
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    residual = output[f->pos];
                    sample1  = output[f->pos-1];
                    output[f->pos] = (int32_t)(sample1 + residual);
                }
                break;
            }
            case 2: {
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    residual = output[f->pos];
                    sample1  = output[f->pos-1];
                    sample2  = output[f->pos-2];

                    sample1 *= 2;

                    output[f->pos] = (int32_t)(sample1 - sample2 + residual);
                }
                break;
            }
            case 3: {
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    residual = output[f->pos];
                    sample1  = output[f->pos-1];
                    sample2  = output[f->pos-2];
                    sample3  = output[f->pos-3];

                    sample1 *= 3;
                    sample2 *= 3;

                    output[f->pos] = (int32_t)(sample1 - sample2 + sample3 + residual);
                }
                break;
            }
            case 4: {
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    residual = output[f->pos];
                    sample1  = output[f->pos-1];
                    sample2  = output[f->pos-2];
                    sample3  = output[f->pos-3];
                    sample4  = output[f->pos-4];

                    sample1 *= 4;
                    sample2 *= 6;
                    sample3 *= 4;

                    output[f->pos] = (int32_t)(sample1  - sample2 + sample3 - sample4 + residual);
                }
                break;
            }
            default: break;
        }
    }

    miniflac_subframe_fixed_init(f);
    return MINIFLAC_OK;

}

MINIFLAC_PRIVATE
void
miniflac_subframe_header_init(miniflac_subframe_header* subframeheader) {
    subframeheader->state       = MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1;
    subframeheader->type        = MINIFLAC_SUBFRAME_TYPE_UNKNOWN;
    subframeheader->order       = 0;
    subframeheader->wasted_bits = 0;
    subframeheader->type_raw    = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_header_decode(miniflac_subframe_header* subframeheader, miniflac_bitreader* br) {
    uint64_t t = 0;
    switch(subframeheader->state) {
        case MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1: {
            if(miniflac_bitreader_fill(br,1)) {
                break;
            }
            t = miniflac_bitreader_read(br,1);
            if(t != 0) {
                miniflac_abort();
                return MINIFLAC_SUBFRAME_RESERVED_BIT;
            }
            subframeheader->state = MINIFLAC_SUBFRAME_HEADER_KIND;
        }
        /* fall-through */
        case MINIFLAC_SUBFRAME_HEADER_KIND: {
            if(miniflac_bitreader_fill(br,6)) {
                break;
            }
            t = (uint8_t)miniflac_bitreader_read(br,6);
            subframeheader->type_raw = t;
            if(t == 0) {
                subframeheader->type = MINIFLAC_SUBFRAME_TYPE_CONSTANT;
            } else if(t == 1) {
                subframeheader->type = MINIFLAC_SUBFRAME_TYPE_VERBATIM;
            } else if(t < 8) {
                miniflac_abort();
                return MINIFLAC_SUBFRAME_RESERVED_TYPE;
            } else if(t < 13) {
                subframeheader->type = MINIFLAC_SUBFRAME_TYPE_FIXED;
                subframeheader->order = t - 8;
            } else if(t < 32) {
                miniflac_abort();
                return MINIFLAC_SUBFRAME_RESERVED_TYPE;
            } else {
                subframeheader->type = MINIFLAC_SUBFRAME_TYPE_LPC;
                subframeheader->order = t - 31;
            }
            subframeheader->state = MINIFLAC_SUBFRAME_HEADER_WASTED_BITS;
        }
        /* fall-through */
        case MINIFLAC_SUBFRAME_HEADER_WASTED_BITS: {
            if(miniflac_bitreader_fill(br,1)) {
                break;
            }
            subframeheader->wasted_bits = 0;

            t = miniflac_bitreader_read(br,1);
            if(t == 0) { /* no wasted bits, we're done */
                subframeheader->state = MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1;
                return MINIFLAC_OK;
            }
            subframeheader->state = MINIFLAC_SUBFRAME_HEADER_UNARY;
        }
        /* fall-through */
        case MINIFLAC_SUBFRAME_HEADER_UNARY: {
            while(!miniflac_bitreader_fill(br,1)) {
                subframeheader->wasted_bits++;
                t = miniflac_bitreader_read(br,1);

                if(t == 1) {
                    /* no more wasted bits, we're done */
                    subframeheader->state = MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1;
                    return MINIFLAC_OK;
                }
            }
        }
        /* fall-through */
        default: break;
    }
    return MINIFLAC_CONTINUE;
}

MINIFLAC_PRIVATE
void
miniflac_subframe_lpc_init(miniflac_subframe_lpc *l) {
    unsigned int i;
    l->pos   = 0;
    l->precision = 0;
    l->shift = 0;
    l->coeff = 0;
    for(i = 0; i < 32; i++) {
        l->coefficients[i] = 0;
    }
    l->state = MINIFLAC_SUBFRAME_LPC_PRECISION;
    miniflac_residual_init(&l->residual);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_lpc_decode(miniflac_subframe_lpc* l, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps, uint8_t predictor_order) {
    int32_t sample;
    int64_t temp;
    int64_t prediction;
    uint32_t i,j;
    MINIFLAC_RESULT r;

    while(l->pos < predictor_order) {
        if(miniflac_bitreader_fill(br,bps)) return MINIFLAC_CONTINUE;
        sample = (int32_t) miniflac_bitreader_read_signed(br,bps);
        if(output != NULL) {
            output[l->pos] = sample;
        }
        l->pos++;
        l->state = MINIFLAC_SUBFRAME_LPC_PRECISION;
    }

    if(l->state == MINIFLAC_SUBFRAME_LPC_PRECISION) {
        if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
        l->precision = miniflac_bitreader_read(br,4) + 1;
        l->state = MINIFLAC_SUBFRAME_LPC_SHIFT;
    }

    if(l->state == MINIFLAC_SUBFRAME_LPC_SHIFT) {
        if(miniflac_bitreader_fill(br,5)) return MINIFLAC_CONTINUE;
        temp = miniflac_bitreader_read_signed(br,5);
        if(temp < 0) temp = 0;
        l->shift = temp;
        l->state = MINIFLAC_SUBFRAME_LPC_COEFF;
    }

    if(l->state == MINIFLAC_SUBFRAME_LPC_COEFF) {
        while(l->coeff < predictor_order) {
            if(miniflac_bitreader_fill(br,l->precision)) return MINIFLAC_CONTINUE;
            sample = (int32_t) miniflac_bitreader_read_signed(br,l->precision);
            l->coefficients[l->coeff++] = sample;
        }
    }

    r = miniflac_residual_decode(&l->residual,br,&l->pos,block_size,predictor_order,output);
    if(r != MINIFLAC_OK) return r;

    if(output != NULL) {
        for(i=predictor_order;i<block_size;i++) {
            prediction = 0;
            for(j=0;j<predictor_order;j++) {
                temp = output[i - j - 1];
                temp *= l->coefficients[j];
                prediction += temp;
            }
            prediction >>= l->shift;
            prediction += output[i];
            output[i] = prediction;
        }
    }

    miniflac_subframe_lpc_init(l);
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
void
miniflac_subframe_verbatim_init(miniflac_subframe_verbatim *c) {
    c->pos   = 0;
    c->state = MINIFLAC_SUBFRAME_VERBATIM_DECODE;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_verbatim_decode(miniflac_subframe_verbatim* c, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps) {
    int32_t sample;

    while(c->pos < block_size) {
        if(miniflac_bitreader_fill(br,bps)) return MINIFLAC_CONTINUE;
        sample = (int32_t) miniflac_bitreader_read_signed(br,bps);
        if(output != NULL) {
            output[c->pos] = sample;
        }
        c->pos++;
    }

    c->pos = 0;
    return MINIFLAC_OK;

}

#endif
