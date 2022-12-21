/* SPDX-License-Identifier: 0BSD */
#include "flac.h"
#include <stddef.h>

#define MINIFLAC_VERSION_MAJOR 1
#define MINIFLAC_VERSION_MINOR 1
#define MINIFLAC_VERSION_PATCH 0

#define MINIFLAC_STR(x) #x
#define MINIFLAC_XSTR(x) MINIFLAC_STR(x)

#define MINIFLAC_VERSION_STRING MINIFLAC_XSTR(MINIFLAC_VERSION_MAJOR) "." MINIFLAC_XSTR(MINIFLAC_VERSION_MINOR) "." MINIFLAC_XSTR(MINIFLAC_VERSION_PATCH)


MINIFLAC_API
unsigned int
miniflac_version_major(void) {
    return MINIFLAC_VERSION_MAJOR;
}

MINIFLAC_API
unsigned int
miniflac_version_minor(void) {
    return MINIFLAC_VERSION_MINOR;
}

MINIFLAC_API
unsigned int
miniflac_version_patch(void) {
    return MINIFLAC_VERSION_PATCH;
}

MINIFLAC_API
const char*
miniflac_version_string(void) {
    return MINIFLAC_VERSION_STRING;
}

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

    while(pFlac->ogg.state != MINIFLAC_OGG_DATA) {
        r = miniflac_ogg_sync(&pFlac->ogg,&pFlac->ogg.br);
        if(r != MINIFLAC_OK) return r;

        if(pFlac->oggserial_set == 0) {
            if(pFlac->ogg.headertype & 0x02) {
                miniflac_oggreset(pFlac);
            }
        } else {
            if(pFlac->oggserial != pFlac->ogg.serialno) pFlac->ogg.state = MINIFLAC_OGG_SKIP;
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
        if(pFlac->ogg.headertype & 0x04) {
            if(pFlac->oggserial_set == 1 && pFlac->oggserial == pFlac->ogg.serialno) {
                pFlac->oggserial_set = 0;
                pFlac->oggserial = 0;
            }
        }
    }
}

MINIFLAC_API
MINIFLAC_CONST
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
    pFlac->oggserial = -1;
    pFlac->oggserial_set = 0;

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
miniflac_sync_internal(miniflac_t* pFlac, miniflac_bitreader_t* br) {
    MINIFLAC_RESULT r;
    unsigned char c;
    uint16_t peek;

    switch(pFlac->state) {
        case MINIFLAC_OGGHEADER: {
            r = miniflac_oggheader_decode(&pFlac->oggheader,br);
            if (r != MINIFLAC_OK) return r;
            pFlac->oggserial_set = 1;
            pFlac->oggserial = pFlac->ogg.serialno;
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
        if(r != MINIFLAC_OK) break;

        r = miniflac_sync_native(pFlac,packet,packet_length,&packet_used);
        miniflac_oggfunction_end(pFlac,packet_used);

        if(r == MINIFLAC_OGG_HEADER_NOTFLAC) {
            /* try reading more data */
            pFlac->ogg.state = MINIFLAC_OGG_SKIP;
            r = MINIFLAC_CONTINUE;
        }
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
uint8_t
miniflac_is_metadata(miniflac_t* pFlac) {
    return pFlac->state == MINIFLAC_METADATA;
}

MINIFLAC_API
uint8_t
miniflac_is_frame(miniflac_t* pFlac) {
    return pFlac->state == MINIFLAC_FRAME;
}

MINIFLAC_API
uint8_t
miniflac_metadata_is_last(miniflac_t* pFlac) {
    return pFlac->metadata.header.is_last;
}

MINIFLAC_API
MINIFLAC_METADATA_TYPE
miniflac_metadata_type(miniflac_t* pFlac) {
    return pFlac->metadata.header.type;
}

MINIFLAC_API
uint32_t
miniflac_metadata_length(miniflac_t* pFlac) {
    return pFlac->metadata.header.length;
}

MINIFLAC_API
uint8_t
miniflac_metadata_is_streaminfo(miniflac_t* pFlac) {
    return pFlac->metadata.header.type == MINIFLAC_METADATA_STREAMINFO;
}

MINIFLAC_API
uint8_t
miniflac_metadata_is_padding(miniflac_t* pFlac) {
    return pFlac->metadata.header.type == MINIFLAC_METADATA_PADDING;
}

MINIFLAC_API
uint8_t
miniflac_metadata_is_application(miniflac_t* pFlac) {
    return pFlac->metadata.header.type == MINIFLAC_METADATA_APPLICATION;
}

MINIFLAC_API
uint8_t
miniflac_metadata_is_seektable(miniflac_t* pFlac) {
    return pFlac->metadata.header.type == MINIFLAC_METADATA_SEEKTABLE;
}

MINIFLAC_API
uint8_t
miniflac_metadata_is_vorbis_comment(miniflac_t* pFlac) {
    return pFlac->metadata.header.type == MINIFLAC_METADATA_VORBIS_COMMENT;
}

MINIFLAC_API
uint8_t
miniflac_metadata_is_cuesheet(miniflac_t* pFlac) {
    return pFlac->metadata.header.type == MINIFLAC_METADATA_CUESHEET;
}

MINIFLAC_API
uint8_t
miniflac_metadata_is_picture(miniflac_t* pFlac) {
    return pFlac->metadata.header.type == MINIFLAC_METADATA_PICTURE;
}

MINIFLAC_API
uint8_t
miniflac_frame_blocking_strategy(miniflac_t* pFlac) {
    return pFlac->frame.header.blocking_strategy;
}

MINIFLAC_API
uint16_t
miniflac_frame_block_size(miniflac_t* pFlac) {
    return pFlac->frame.header.block_size;
}

MINIFLAC_API
uint32_t
miniflac_frame_sample_rate(miniflac_t* pFlac) {
    return pFlac->frame.header.sample_rate;
}

MINIFLAC_API
uint8_t
miniflac_frame_channels(miniflac_t* pFlac) {
    return pFlac->frame.header.channels;
}

MINIFLAC_API
uint8_t
miniflac_frame_bps(miniflac_t* pFlac) {
    return pFlac->frame.header.bps;
}

MINIFLAC_API
uint64_t
miniflac_frame_sample_number(miniflac_t* pFlac) {
    return pFlac->frame.header.sample_number;
}

MINIFLAC_API
uint32_t
miniflac_frame_frame_number(miniflac_t* pFlac) {
    return pFlac->frame.header.frame_number;
}

#define MINIFLAC_SUBSYS(subsys) &pFlac->metadata.subsys

#define MINIFLAC_GEN_NATIVE_FUNC1(mt,subsys,val,t) \
static \
MINIFLAC_RESULT \
miniflac_ ## subsys ## _ ## val ## _native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, t* outvar) { \
    MINIFLAC_RESULT r; \
    pFlac->br.buffer = data; \
    pFlac->br.len    = length; \
    pFlac->br.pos    = 0; \
    while(pFlac->state != MINIFLAC_METADATA) { \
        r = miniflac_sync_internal(pFlac,&pFlac->br); \
        if(r != MINIFLAC_OK) goto miniflac_ ## subsys ## _ ## val ##_exit; \
    } \
    while(pFlac->metadata.header.type != MINIFLAC_METADATA_ ## mt) { \
        r = miniflac_sync_internal(pFlac,&pFlac->br); \
        if(r != MINIFLAC_OK) goto miniflac_## subsys ## _ ## val ## _exit; \
        if(pFlac->state != MINIFLAC_METADATA) { \
            r = MINIFLAC_ERROR; \
            goto miniflac_ ## subsys ## _ ## val ## _exit; \
        } \
    } \
    r = miniflac_ ## subsys ## _read_ ## val(MINIFLAC_SUBSYS(subsys),&pFlac->br, outvar); \
    miniflac_ ## subsys ## _ ## val ## _exit: \
    *out_length = pFlac->br.pos; \
    return r; \
}

#define MINIFLAC_GEN_OGG_FUNC1(subsys,val,t) \
static \
MINIFLAC_RESULT \
miniflac_ ## subsys ## _ ## val ## _ogg(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, t* outvar) { \
    MINIFLAC_RESULT r = MINIFLAC_CONTINUE; \
    const uint8_t* packet = NULL; \
    uint32_t packet_length = 0; \
    uint32_t packet_used   = 0; \
    pFlac->ogg.br.buffer = data; \
    pFlac->ogg.br.len = length; \
    pFlac->ogg.br.pos = 0; \
    do { \
        r = miniflac_oggfunction_start(pFlac,data,&packet,&packet_length); \
        if(r  != MINIFLAC_OK) break; \
        r = miniflac_ ## subsys ## _ ## val ## _native(pFlac,packet,packet_length,&packet_used,outvar); \
        miniflac_oggfunction_end(pFlac,packet_used); \
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length); \
    *out_length = pFlac->ogg.br.pos; \
    return r; \
} \

#define MINIFLAC_GEN_FUNC1(mt,subsys,val,t) \
MINIFLAC_GEN_NATIVE_FUNC1(mt,subsys,val,t) \
MINIFLAC_GEN_OGG_FUNC1(subsys,val,t) \
MINIFLAC_API \
MINIFLAC_RESULT \
miniflac_ ## subsys ## _ ## val(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, t* outvar) { \
  MINIFLAC_RESULT r; \
  if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) { \
        r = miniflac_probe(pFlac,data,length); \
        if(r != MINIFLAC_OK) return r; \
    } \
    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) { \
        r = miniflac_ ## subsys ## _ ## val ## _native(pFlac,data,length,out_length,outvar); \
    } else { \
        r = miniflac_ ## subsys ## _ ## val ## _ogg(pFlac,data,length,out_length,outvar); \
    } \
    return r; \
}

#define MINIFLAC_GEN_NATIVE_FUNCSTR(mt,subsys,val,t) \
static \
MINIFLAC_RESULT \
miniflac_ ## subsys ## _ ## val ## _native(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, t* buffer, uint32_t bufferlen, uint32_t* outlen)  { \
    MINIFLAC_RESULT r; \
    pFlac->br.buffer = data; \
    pFlac->br.len    = length; \
    pFlac->br.pos    = 0; \
    while(pFlac->state != MINIFLAC_METADATA) { \
        r = miniflac_sync_internal(pFlac,&pFlac->br); \
        if(r != MINIFLAC_OK) goto miniflac_ ## subsys ## _ ## val ##_exit; \
    } \
    while(pFlac->metadata.header.type != MINIFLAC_METADATA_ ## mt) { \
        r = miniflac_sync_internal(pFlac,&pFlac->br); \
        if(r != MINIFLAC_OK) goto miniflac_## subsys ## _ ## val ## _exit; \
        if(pFlac->state != MINIFLAC_METADATA) { \
            r = MINIFLAC_ERROR; \
            goto miniflac_ ## subsys ## _ ## val ## _exit; \
        } \
    } \
    r = miniflac_ ## subsys ## _read_ ## val(MINIFLAC_SUBSYS(subsys),&pFlac->br, buffer, bufferlen, outlen); \
    miniflac_ ## subsys ## _ ## val ## _exit: \
    *out_length = pFlac->br.pos; \
    return r; \
}

#define MINIFLAC_GEN_OGG_FUNCSTR(subsys,val,t) \
static \
MINIFLAC_RESULT \
miniflac_ ## subsys ## _ ## val ## _ogg(miniflac_t *pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, t* buffer, uint32_t bufferlen, uint32_t* outlen)  { \
    MINIFLAC_RESULT r = MINIFLAC_CONTINUE; \
    const uint8_t* packet = NULL; \
    uint32_t packet_length = 0; \
    uint32_t packet_used   = 0; \
    pFlac->ogg.br.buffer = data; \
    pFlac->ogg.br.len = length; \
    pFlac->ogg.br.pos = 0; \
    do { \
        r = miniflac_oggfunction_start(pFlac,data,&packet,&packet_length); \
        if(r  != MINIFLAC_OK) break; \
        r = miniflac_ ## subsys ## _ ## val ##_native(pFlac,packet,packet_length,&packet_used,buffer,bufferlen,outlen); \
        miniflac_oggfunction_end(pFlac,packet_used); \
    } while(r == MINIFLAC_CONTINUE && pFlac->ogg.br.pos < length); \
    *out_length = pFlac->ogg.br.pos; \
    return r; \
} \

#define MINIFLAC_GEN_FUNCSTR(mt,subsys,val,t) \
MINIFLAC_GEN_NATIVE_FUNCSTR(mt,subsys,val,t) \
MINIFLAC_GEN_OGG_FUNCSTR(subsys,val,t) \
MINIFLAC_API \
MINIFLAC_RESULT \
miniflac_ ## subsys ## _ ## val(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, t* output, uint32_t buffer_length, uint32_t* outlen) { \
  MINIFLAC_RESULT r; \
  if(pFlac->container == MINIFLAC_CONTAINER_UNKNOWN) { \
        r = miniflac_probe(pFlac,data,length); \
        if(r != MINIFLAC_OK) return r; \
    } \
    if(pFlac->container == MINIFLAC_CONTAINER_NATIVE) { \
        r = miniflac_ ## subsys ## _ ## val ## _native(pFlac,data,length,out_length,output,buffer_length,outlen); \
    } else { \
        r = miniflac_ ## subsys ## _ ## val ## _ogg(pFlac,data,length,out_length,output,buffer_length,outlen); \
    } \
    return r; \
}

MINIFLAC_GEN_FUNC1(STREAMINFO,streaminfo,min_block_size,uint16_t)
MINIFLAC_GEN_FUNC1(STREAMINFO,streaminfo,max_block_size,uint16_t)
MINIFLAC_GEN_FUNC1(STREAMINFO,streaminfo,min_frame_size,uint32_t)
MINIFLAC_GEN_FUNC1(STREAMINFO,streaminfo,max_frame_size,uint32_t)
MINIFLAC_GEN_FUNC1(STREAMINFO,streaminfo,sample_rate,uint32_t)
MINIFLAC_GEN_FUNC1(STREAMINFO,streaminfo,channels,uint8_t)
MINIFLAC_GEN_FUNC1(STREAMINFO,streaminfo,bps,uint8_t)
MINIFLAC_GEN_FUNC1(STREAMINFO,streaminfo,total_samples,uint64_t)
MINIFLAC_GEN_FUNC1(STREAMINFO,streaminfo,md5_length,uint32_t)
MINIFLAC_GEN_FUNCSTR(STREAMINFO,streaminfo,md5_data,uint8_t)

MINIFLAC_GEN_FUNC1(VORBIS_COMMENT,vorbis_comment,vendor_length,uint32_t)
MINIFLAC_GEN_FUNCSTR(VORBIS_COMMENT,vorbis_comment,vendor_string,char)
MINIFLAC_GEN_FUNC1(VORBIS_COMMENT,vorbis_comment,total,uint32_t)
MINIFLAC_GEN_FUNC1(VORBIS_COMMENT,vorbis_comment,length,uint32_t)
MINIFLAC_GEN_FUNCSTR(VORBIS_COMMENT,vorbis_comment,string,char)

MINIFLAC_GEN_FUNC1(PICTURE,picture,type,uint32_t)
MINIFLAC_GEN_FUNC1(PICTURE,picture,mime_length,uint32_t)
MINIFLAC_GEN_FUNCSTR(PICTURE,picture,mime_string,char)
MINIFLAC_GEN_FUNC1(PICTURE,picture,description_length,uint32_t)
MINIFLAC_GEN_FUNCSTR(PICTURE,picture,description_string,char)
MINIFLAC_GEN_FUNC1(PICTURE,picture,width,uint32_t)
MINIFLAC_GEN_FUNC1(PICTURE,picture,height,uint32_t)
MINIFLAC_GEN_FUNC1(PICTURE,picture,colordepth,uint32_t)
MINIFLAC_GEN_FUNC1(PICTURE,picture,totalcolors,uint32_t)
MINIFLAC_GEN_FUNC1(PICTURE,picture,length,uint32_t)
MINIFLAC_GEN_FUNCSTR(PICTURE,picture,data,uint8_t)

MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,catalog_length,uint32_t)
MINIFLAC_GEN_FUNCSTR(CUESHEET,cuesheet,catalog_string,char)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,leadin,uint64_t)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,cd_flag,uint8_t)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,tracks,uint8_t)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,track_offset,uint64_t)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,track_number,uint8_t)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,track_isrc_length,uint32_t)
MINIFLAC_GEN_FUNCSTR(CUESHEET,cuesheet,track_isrc_string,char)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,track_audio_flag,uint8_t)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,track_preemph_flag,uint8_t)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,track_indexpoints,uint8_t)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,index_point_offset,uint64_t)
MINIFLAC_GEN_FUNC1(CUESHEET,cuesheet,index_point_number,uint8_t)

MINIFLAC_GEN_FUNC1(SEEKTABLE,seektable,seekpoints,uint32_t)
MINIFLAC_GEN_FUNC1(SEEKTABLE,seektable,sample_number,uint64_t)
MINIFLAC_GEN_FUNC1(SEEKTABLE,seektable,sample_offset,uint64_t)
MINIFLAC_GEN_FUNC1(SEEKTABLE,seektable,samples,uint16_t)

MINIFLAC_GEN_FUNC1(APPLICATION,application,id,uint32_t)
MINIFLAC_GEN_FUNC1(APPLICATION,application,length,uint32_t)
MINIFLAC_GEN_FUNCSTR(APPLICATION,application,data,uint8_t)

MINIFLAC_GEN_FUNC1(PADDING,padding,length,uint32_t)
MINIFLAC_GEN_FUNCSTR(PADDING,padding,data,uint8_t)
