/* SPDX-License-Identifier: 0BSD */
#include "metadata.h"
#include <assert.h>

MINIFLAC_PRIVATE
void
miniflac_metadata_init(miniflac_metadata* metadata) {
    metadata->state = MINIFLAC_METADATA_HEADER;
    metadata->pos = 0;
    miniflac_metadata_header_init(&metadata->header);
    miniflac_streaminfo_init(&metadata->streaminfo);
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
