/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_METADATA_HEADER_H
#define MINIFLAC_METADATA_HEADER_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

enum MINIFLAC_METADATA_TYPE {
    MINIFLAC_METADATA_STREAMINFO     = 0,
    MINIFLAC_METADATA_PADDING        = 1,
    MINIFLAC_METADATA_APPLICATION    = 2,
    MINIFLAC_METADATA_SEEKTABLE      = 3,
    MINIFLAC_METADATA_VORBIS_COMMENT = 4,
    MINIFLAC_METADATA_CUESHEET       = 5,
    MINIFLAC_METADATA_PICTURE        = 6,
    MINIFLAC_METADATA_INVALID      = 127,
    MINIFLAC_METADATA_UNKNOWN      = 128,
};

enum MINIFLAC_METADATA_HEADER_STATE {
    MINIFLAC_METADATA_LAST_FLAG,
    MINIFLAC_METADATA_BLOCK_TYPE,
    MINIFLAC_METADATA_LENGTH,
};

struct miniflac_metadata_header_s {
    enum MINIFLAC_METADATA_HEADER_STATE    state;
    uint8_t                         is_last;
    uint8_t                        type_raw;
    enum MINIFLAC_METADATA_TYPE             type;
    uint32_t                         length;
};

typedef struct miniflac_metadata_header_s miniflac_metadata_header_t;
typedef enum MINIFLAC_METADATA_TYPE MINIFLAC_METADATA_TYPE;
typedef enum MINIFLAC_METADATA_HEADER_STATE MINIFLAC_METADATA_HEADER_STATE;


#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_metadata_header_init(miniflac_metadata_header_t* header);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_header_decode(miniflac_metadata_header_t* header, miniflac_bitreader_t* br);

#ifdef __cplusplus
}
#endif

#endif



