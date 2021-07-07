/* SPDX-License-Identifier: 0BSD */
#include "flac.h"
#include <stddef.h>

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

MINIFLAC_PRIVATE
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

MINIFLAC_PRIVATE
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

MINIFLAC_PRIVATE
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
        if(pFlac->ogg.state != MINIFLAC_OGG_DATA) {
            r = miniflac_ogg_sync(&pFlac->ogg,&pFlac->ogg.br);
            if(r != MINIFLAC_OK) break;

            if(pFlac->ogg.headertype & 0x02) {
                miniflac_bitreader_init(&pFlac->br);
                miniflac_oggheader_init(&pFlac->oggheader);
                miniflac_streammarker_init(&pFlac->streammarker);
                miniflac_metadata_init(&pFlac->metadata);
                miniflac_frame_init(&pFlac->frame);
                pFlac->state = MINIFLAC_OGGHEADER;
            }
        }

        packet = &data[pFlac->ogg.br.pos];
        packet_length = pFlac->ogg.br.len - pFlac->ogg.br.pos;
        if(packet_length > (uint32_t)(pFlac->ogg.length - pFlac->ogg.pos)) {
            packet_length = (uint32_t)pFlac->ogg.length - pFlac->ogg.pos;
        }

        r = miniflac_sync_native(pFlac,packet,packet_length,&packet_used);
        pFlac->ogg.br.pos += packet_used;

        pFlac->ogg.pos    += packet_used;
        if(pFlac->ogg.pos == pFlac->ogg.length) {
            pFlac->ogg.state = MINIFLAC_OGG_CAPTUREPATTERN_O;
        }
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

MINIFLAC_PRIVATE
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
        if(pFlac->ogg.state != MINIFLAC_OGG_DATA) {
            r = miniflac_ogg_sync(&pFlac->ogg,&pFlac->ogg.br);
            if(r != MINIFLAC_OK) break;
            if(pFlac->ogg.headertype & 0x02) {
                miniflac_bitreader_init(&pFlac->br);
                miniflac_oggheader_init(&pFlac->oggheader);
                miniflac_streammarker_init(&pFlac->streammarker);
                miniflac_metadata_init(&pFlac->metadata);
                miniflac_frame_init(&pFlac->frame);
                pFlac->state = MINIFLAC_OGGHEADER;
            }
        }

        packet = &data[pFlac->ogg.br.pos];
        packet_length = pFlac->ogg.br.len - pFlac->ogg.br.pos;
        if(packet_length > (uint32_t)(pFlac->ogg.length - pFlac->ogg.pos)) {
            packet_length = (uint32_t)pFlac->ogg.length - pFlac->ogg.pos;
        }

        r = miniflac_decode_native(pFlac,packet,packet_length,&packet_used,samples);
        pFlac->ogg.br.pos += packet_used;
        pFlac->ogg.pos    += packet_used;

        if(pFlac->ogg.pos == pFlac->ogg.length) {
            pFlac->ogg.state = MINIFLAC_OGG_CAPTUREPATTERN_O;
        }
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length);

    *out_length = pFlac->ogg.br.pos;
    return r;
}

MINIFLAC_PRIVATE
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
