/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_METADATA_H
#define MINIFLAC_METADATA_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"
#include "metadataheader.h"
#include "streaminfo.h"
#include "vorbiscomment.h"
#include "picture.h"
#include "cuesheet.h"
#include "seektable.h"
#include "application.h"
#include "padding.h"

typedef struct miniflac_metadata_s miniflac_metadata_t;
typedef enum MINIFLAC_METADATA_STATE MINIFLAC_METADATA_STATE;

enum MINIFLAC_METADATA_STATE {
    MINIFLAC_METADATA_HEADER,
    MINIFLAC_METADATA_DATA,
};

struct miniflac_metadata_s {
    MINIFLAC_METADATA_STATE               state;
    uint32_t                                pos;
    miniflac_metadata_header_t           header;
    miniflac_streaminfo_t            streaminfo;
    miniflac_vorbis_comment_t    vorbis_comment;
    miniflac_picture_t                  picture;
    miniflac_cuesheet_t                cuesheet;
    miniflac_seektable_t              seektable;
    miniflac_application_t          application;
    miniflac_padding_t                  padding;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_metadata_init(miniflac_metadata_t* metadata);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_sync(miniflac_metadata_t* metadata, miniflac_bitreader_t* br);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_decode(miniflac_metadata_t* metadata, miniflac_bitreader_t* br);

#ifdef __cplusplus
}
#endif

#endif


