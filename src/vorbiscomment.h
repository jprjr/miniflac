/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_VORBIS_COMMENT_H
#define MINIFLAC_VORBIS_COMMENT_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_vorbis_comment_s miniflac_vorbis_comment_t;
typedef enum MINIFLAC_VORBISCOMMENT_STATE MINIFLAC_VORBISCOMMENT_STATE;

enum MINIFLAC_VORBISCOMMENT_STATE {
    MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH,
    MINIFLAC_VORBISCOMMENT_VENDOR_STRING,
    MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS,
    MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH,
    MINIFLAC_VORBISCOMMENT_COMMENT_STRING,
};

struct miniflac_vorbis_comment_s {
    MINIFLAC_VORBISCOMMENT_STATE    state;
    uint32_t len; /* length of the current string we're decoding */
    uint32_t pos; /* position within current string */
    uint32_t tot; /* total comments */
    uint32_t cur; /* current comment being decoded */
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_vorbis_comment_init(miniflac_vorbis_comment_t* vorbis_comment);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_vendor_length(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_vendor_string(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_total(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, uint32_t* total);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_length(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_string(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen);

#ifdef __cplusplus
}
#endif

#endif



