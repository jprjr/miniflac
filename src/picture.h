/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_PICTURE_H
#define MINIFLAC_PICTURE_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_picture_s miniflac_picture_t;
typedef enum MINIFLAC_PICTURE_STATE MINIFLAC_PICTURE_STATE;

enum MINIFLAC_PICTURE_STATE {
    MINIFLAC_PICTURE_TYPE,
    MINIFLAC_PICTURE_MIME_LENGTH,
    MINIFLAC_PICTURE_MIME_STRING,
    MINIFLAC_PICTURE_DESCRIPTION_LENGTH,
    MINIFLAC_PICTURE_DESCRIPTION_STRING,
    MINIFLAC_PICTURE_WIDTH,
    MINIFLAC_PICTURE_HEIGHT,
    MINIFLAC_PICTURE_COLORDEPTH,
    MINIFLAC_PICTURE_TOTALCOLORS,
    MINIFLAC_PICTURE_PICTURE_LENGTH,
    MINIFLAC_PICTURE_PICTURE_DATA,
};

struct miniflac_picture_s {
    MINIFLAC_PICTURE_STATE    state;
    uint32_t len; /* length of the current string/data we're decoding */
    uint32_t pos; /* position within current string */
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_picture_init(miniflac_picture_t* picture);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_type(miniflac_picture_t* picture, miniflac_bitreader_t* br, uint32_t* type);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_mime_length(miniflac_picture_t* picture, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_mime_string(miniflac_picture_t* picture, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_description_length(miniflac_picture_t* picture, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_description_string(miniflac_picture_t* picture, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_width(miniflac_picture_t* picture, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_height(miniflac_picture_t* picture, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_colordepth(miniflac_picture_t* picture, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_totalcolors(miniflac_picture_t* picture, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_picture_length(miniflac_picture_t* picture, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_picture_data(miniflac_picture_t* picture, miniflac_bitreader_t* br, uint8_t* output, uint32_t length, uint32_t* outlen);

#ifdef __cplusplus
}
#endif

#endif




