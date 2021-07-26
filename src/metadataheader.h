/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_METADATA_HEADER_H
#define MINIFLAC_METADATA_HEADER_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_metadata_header_s miniflac_metadata_header;
typedef enum MINIFLAC_METADATA_TYPE MINIFLAC_METADATA_TYPE;
typedef enum MINIFLAC_METADATA_HEADER_STATE MINIFLAC_METADATA_HEADER_STATE;

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

struct miniflac_metadata_header_s {
    MINIFLAC_METADATA_HEADER_STATE    state;
    uint8_t                        is_last;
    uint8_t                       type_raw;
    MINIFLAC_METADATA_TYPE             type;
    uint32_t                        length;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_metadata_header_init(miniflac_metadata_header* header);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_header_decode(miniflac_metadata_header* header, miniflac_bitreader_t* br);

#ifdef __cplusplus
}
#endif

#endif



