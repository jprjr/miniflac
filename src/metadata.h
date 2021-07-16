/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_METADATA_H
#define MINIFLAC_METADATA_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"
#include "metadataheader.h"
#include "streaminfo.h"
#include "vorbiscomment.h"

typedef struct miniflac_metadata_s miniflac_metadata;
typedef enum MINIFLAC_METADATA_STATE MINIFLAC_METADATA_STATE;

enum MINIFLAC_METADATA_STATE {
    MINIFLAC_METADATA_HEADER,
    MINIFLAC_METADATA_DATA,
};

struct miniflac_metadata_s {
    MINIFLAC_METADATA_STATE          state;
    uint32_t                           pos;
    miniflac_metadata_header        header;
    miniflac_streaminfo_t       streaminfo;
    miniflac_vorbiscomment_t vorbiscomment;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_metadata_init(miniflac_metadata* metadata);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_sync(miniflac_metadata* metadata, miniflac_bitreader* br);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_decode(miniflac_metadata* metadata, miniflac_bitreader* br);

#ifdef __cplusplus
}
#endif

#endif


