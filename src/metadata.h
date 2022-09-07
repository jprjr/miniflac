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

enum MINIFLAC_METADATA_STATE {
    MINIFLAC_METADATA_HEADER,
    MINIFLAC_METADATA_DATA,
};

struct miniflac_metadata_s {
    enum MINIFLAC_METADATA_STATE               state;
    uint32_t                                     pos;
    struct miniflac_metadata_header_s         header;
    struct miniflac_streaminfo_s          streaminfo;
    struct miniflac_vorbis_comment_s  vorbis_comment;
    struct miniflac_picture_s                picture;
    struct miniflac_cuesheet_s              cuesheet;
    struct miniflac_seektable_s            seektable;
    struct miniflac_application_s        application;
    struct miniflac_padding_s                padding;
};

typedef struct miniflac_metadata_s miniflac_metadata_t;
typedef enum MINIFLAC_METADATA_STATE MINIFLAC_METADATA_STATE;

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


