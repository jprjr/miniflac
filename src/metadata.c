/* SPDX-License-Identifier: 0BSD */
#include "metadata.h"
#include <assert.h>
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_metadata_init(miniflac_metadata_t* metadata) {
    metadata->state = MINIFLAC_METADATA_HEADER;
    metadata->pos = 0;
    miniflac_metadata_header_init(&metadata->header);
    miniflac_streaminfo_init(&metadata->streaminfo);
    miniflac_vorbis_comment_init(&metadata->vorbis_comment);
    miniflac_picture_init(&metadata->picture);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_sync(miniflac_metadata_t* metadata, miniflac_bitreader_t* br) {
    MINIFLAC_RESULT r;
    assert(metadata->state == MINIFLAC_METADATA_HEADER);
    r = miniflac_metadata_header_decode(&metadata->header,br);
    if(r != MINIFLAC_OK) return r;

    switch(metadata->header.type) {
        case MINIFLAC_METADATA_STREAMINFO: {
            miniflac_streaminfo_init(&metadata->streaminfo);
            break;
        }
        case MINIFLAC_METADATA_VORBIS_COMMENT: {
            miniflac_vorbis_comment_init(&metadata->vorbis_comment);
            break;
        }
        case MINIFLAC_METADATA_PICTURE: {
            miniflac_picture_init(&metadata->picture);
            break;
        }
        case MINIFLAC_METADATA_CUESHEET: {
            miniflac_cuesheet_init(&metadata->cuesheet);
            break;
        }
        default: break;
    }

    metadata->state = MINIFLAC_METADATA_DATA;
    metadata->pos = 0;
    return MINIFLAC_OK;
}

static
MINIFLAC_RESULT
miniflac_metadata_skip(miniflac_metadata_t* metadata, miniflac_bitreader_t* br) {
    while(metadata->pos < metadata->header.length) {
        if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
        miniflac_bitreader_discard(br,8);
        metadata->pos++;
    }
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_decode(miniflac_metadata_t* metadata, miniflac_bitreader_t* br) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint8_t indexpoints = 0;
    switch(metadata->state) {
        case MINIFLAC_METADATA_HEADER: {
            r = miniflac_metadata_sync(metadata,br);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_METADATA_DATA: {
            switch(metadata->header.type) {
                case MINIFLAC_METADATA_STREAMINFO: {
                    r = miniflac_streaminfo_read_streaminfo(&metadata->streaminfo,br,NULL);
                    break;
                }
                case MINIFLAC_METADATA_VORBIS_COMMENT: {
                    do {
                        r = miniflac_vorbis_comment_read_length(&metadata->vorbis_comment,br,NULL);
                    } while(r == MINIFLAC_OK);
                    break;
                }
                case MINIFLAC_METADATA_PICTURE: {
                    r = miniflac_picture_read_data(&metadata->picture,br,NULL,0,NULL);
                    break;
                }
                case MINIFLAC_METADATA_CUESHEET: {
                    do {
                      r = miniflac_cuesheet_read_track_indexpoints(&metadata->cuesheet,br,&indexpoints);
                    } while(r == MINIFLAC_OK);
                    break;
                }
                default: {
                    r = miniflac_metadata_skip(metadata,br);
                }
            }
        }
        /* fall-through */
        default: break;
    }

    if(r == MINIFLAC_METADATA_END) {
        r = MINIFLAC_OK;
    }

    if(r != MINIFLAC_OK) return r;

    assert(br->bits == 0);
    br->crc8  = 0;
    br->crc16 = 0;
    metadata->state = MINIFLAC_METADATA_HEADER;
    metadata->pos   = 0;
    return MINIFLAC_OK;
}
