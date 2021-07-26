/* SPDX-License-Identifier: 0BSD */
#include "flac.h"
#include <stddef.h>

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
miniflac_streaminfo_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, miniflac_streaminfo_t* streaminfo) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_streaminfo_decode_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_STREAMINFO) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_streaminfo_decode_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_streaminfo_decode_exit;
        }
    }

    r = miniflac_streaminfo_decode(&pFlac->metadata.streaminfo,&pFlac->br,streaminfo);
    /* perform some housekeeping for the metadata_decode/sync functions, minor duplication but this is a special-case block */

    miniflac_streaminfo_decode_exit:
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
miniflac_picture_type_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_type) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_type_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_type_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_type_exit;
        }
    }

    r = miniflac_picture_read_type(&pFlac->metadata.picture,&pFlac->br,picture_type);

    miniflac_picture_type_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_mime_length_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_mime_length) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_mime_length_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_mime_length_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_mime_length_exit;
        }
    }

    r = miniflac_picture_read_mime_length(&pFlac->metadata.picture,&pFlac->br,picture_mime_length);

    miniflac_picture_mime_length_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_mime_string_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_mime_string_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_mime_string_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_mime_string_exit;
        }
    }

    r = miniflac_picture_read_mime_string(&pFlac->metadata.picture,&pFlac->br,buffer, buffer_length, buffer_used);

    miniflac_picture_mime_string_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_description_length_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_description_length) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_description_length_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_description_length_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_description_length_exit;
        }
    }

    r = miniflac_picture_read_description_length(&pFlac->metadata.picture,&pFlac->br,picture_description_length);

    miniflac_picture_description_length_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_description_string_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_description_string_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_description_string_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_description_string_exit;
        }
    }

    r = miniflac_picture_read_description_string(&pFlac->metadata.picture,&pFlac->br,buffer, buffer_length, buffer_used);

    miniflac_picture_description_string_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_width_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_width) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_width_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_width_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_width_exit;
        }
    }

    r = miniflac_picture_read_width(&pFlac->metadata.picture,&pFlac->br,picture_width);

    miniflac_picture_width_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_height_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_height) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_height_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_height_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_height_exit;
        }
    }

    r = miniflac_picture_read_height(&pFlac->metadata.picture,&pFlac->br,picture_height);

    miniflac_picture_height_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_colordepth_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_colordepth) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_colordepth_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_colordepth_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_colordepth_exit;
        }
    }

    r = miniflac_picture_read_colordepth(&pFlac->metadata.picture,&pFlac->br,picture_colordepth);

    miniflac_picture_colordepth_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_totalcolors_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_totalcolors) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_totalcolors_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_totalcolors_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_totalcolors_exit;
        }
    }

    r = miniflac_picture_read_totalcolors(&pFlac->metadata.picture,&pFlac->br,picture_totalcolors);

    miniflac_picture_totalcolors_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_length_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_length) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_length_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_length_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_length_exit;
        }
    }

    r = miniflac_picture_read_picture_length(&pFlac->metadata.picture,&pFlac->br,picture_length);

    miniflac_picture_length_exit:
    *out_length = pFlac->br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_data_native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    pFlac->br.buffer = data;
    pFlac->br.len    = length;
    pFlac->br.pos    = 0;

    while(pFlac->state != MINIFLAC_METADATA) {
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_data_exit;
    }

    while(pFlac->metadata.header.type != MINIFLAC_METADATA_PICTURE) {
        /* sync to the next block */
        r = miniflac_sync_internal(pFlac,&pFlac->br);
        if(r != MINIFLAC_OK) goto miniflac_picture_data_exit;
        if(pFlac->state != MINIFLAC_METADATA) {
            r = MINIFLAC_ERROR;
            goto miniflac_picture_data_exit;
        }
    }

    r = miniflac_picture_read_picture_data(&pFlac->metadata.picture,&pFlac->br,buffer, buffer_length, buffer_used);

    miniflac_picture_data_exit:
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
miniflac_streaminfo_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, miniflac_streaminfo_t* streaminfo) {
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

        r = miniflac_streaminfo_native(pFlac,packet,packet_length,&packet_used,streaminfo);
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
miniflac_picture_type_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_type) {
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

        r = miniflac_picture_type_native(pFlac,packet,packet_length,&packet_used,picture_type);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_mime_length_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_mime_length) {
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

        r = miniflac_picture_mime_length_native(pFlac,packet,packet_length,&packet_used,picture_mime_length);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_mime_string_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
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

        r = miniflac_picture_mime_string_native(pFlac,packet,packet_length,&packet_used,buffer,buffer_length,buffer_used);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_description_length_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_description_length) {
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

        r = miniflac_picture_description_length_native(pFlac,packet,packet_length,&packet_used,picture_description_length);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_description_string_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
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

        r = miniflac_picture_description_string_native(pFlac,packet,packet_length,&packet_used,buffer,buffer_length,buffer_used);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_width_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_width) {
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

        r = miniflac_picture_width_native(pFlac,packet,packet_length,&packet_used,picture_width);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_height_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_height) {
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

        r = miniflac_picture_height_native(pFlac,packet,packet_length,&packet_used,picture_height);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_colordepth_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_colordepth) {
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

        r = miniflac_picture_colordepth_native(pFlac,packet,packet_length,&packet_used,picture_colordepth);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_totalcolors_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_totalcolors) {
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

        r = miniflac_picture_totalcolors_native(pFlac,packet,packet_length,&packet_used,picture_totalcolors);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_length_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_length) {
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

        r = miniflac_picture_length_native(pFlac,packet,packet_length,&packet_used,picture_length);
        miniflac_oggfunction_end(pFlac,packet_used);
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

static
MINIFLAC_RESULT
miniflac_picture_data_ogg(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
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

        r = miniflac_picture_data_native(pFlac,packet,packet_length,&packet_used,buffer,buffer_length,buffer_used);
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
miniflac_streaminfo(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, miniflac_streaminfo_t* streaminfo) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_streaminfo_native(pFlac,data,length,out_length,streaminfo);
    } else {
        r = miniflac_streaminfo_ogg(pFlac,data,length,out_length,streaminfo);
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

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_type(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_type) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_type_native(pFlac,data,length,out_length,picture_type);
    } else {
        r = miniflac_picture_type_ogg(pFlac,data,length,out_length,picture_type);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_mime_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_mime_length) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_mime_length_native(pFlac,data,length,out_length,picture_mime_length);
    } else {
        r = miniflac_picture_mime_length_ogg(pFlac,data,length,out_length,picture_mime_length);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_mime_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_mime_string_native(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    } else {
        r = miniflac_picture_mime_string_ogg(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_description_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_description_length) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_description_length_native(pFlac,data,length,out_length,picture_description_length);
    } else {
        r = miniflac_picture_description_length_ogg(pFlac,data,length,out_length,picture_description_length);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_description_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_description_string_native(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    } else {
        r = miniflac_picture_description_string_ogg(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_width(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_width) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_width_native(pFlac,data,length,out_length,picture_width);
    } else {
        r = miniflac_picture_width_ogg(pFlac,data,length,out_length,picture_width);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_height(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_height) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_height_native(pFlac,data,length,out_length,picture_height);
    } else {
        r = miniflac_picture_height_ogg(pFlac,data,length,out_length,picture_height);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_colordepth(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_colordepth) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_colordepth_native(pFlac,data,length,out_length,picture_colordepth);
    } else {
        r = miniflac_picture_colordepth_ogg(pFlac,data,length,out_length,picture_colordepth);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_totalcolors(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_totalcolors) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_totalcolors_native(pFlac,data,length,out_length,picture_totalcolors);
    } else {
        r = miniflac_picture_totalcolors_ogg(pFlac,data,length,out_length,picture_totalcolors);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_length) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_length_native(pFlac,data,length,out_length,picture_length);
    } else {
        r = miniflac_picture_length_ogg(pFlac,data,length,out_length,picture_length);
    }

    return r;
}

MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_data(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* buffer, uint32_t buffer_length, uint32_t* buffer_used) {
    MINIFLAC_RESULT r;
    if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) {
        r = miniflac_probe(pFlac,data,length);
        if(r != MINIFLAC_OK) return r;
    }

    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) {
        r = miniflac_picture_data_native(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    } else {
        r = miniflac_picture_data_ogg(pFlac,data,length,out_length,buffer, buffer_length, buffer_used);
    }

    return r;
}
