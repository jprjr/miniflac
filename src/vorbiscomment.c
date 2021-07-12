#include "vorbiscomment.h"
#include "unpack.h"

#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_vorbiscomment_init(miniflac_vorbiscomment_t* vorbiscomment) {
    vorbiscomment->state = MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH;
    vorbiscomment->len = 0;
    vorbiscomment->pos = 0;
    vorbiscomment->tot = 0;
    vorbiscomment->cur = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_vendor_length(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, uint32_t* length) {
    uint8_t buffer[4];
    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            vorbiscomment->len = miniflac_unpack_uint32le(buffer);
            if(length != NULL) *length = vorbiscomment->len;
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_VENDOR_STRING;
            return MINIFLAC_OK;
        } default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_vendor_string(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, char* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    char c;

    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: {
            r = miniflac_vorbiscomment_vendor_length(vorbiscomment,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING: {
            while(vorbiscomment->pos < vorbiscomment->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && vorbiscomment->pos < length) {
                    output[vorbiscomment->pos] = c;
                }
                vorbiscomment->pos++;
            }
            if(outlen != NULL) {
                *outlen = vorbiscomment->len <= length ? vorbiscomment->len : length;
            }
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_total_comments(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, uint32_t* total) {
    uint8_t buffer[4];
    MINIFLAC_RESULT r = MINIFLAC_ERROR;

    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: /* fall-through */
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING: {
            r = miniflac_vorbiscomment_vendor_string(vorbiscomment,br,NULL,0,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            vorbiscomment->tot = miniflac_unpack_uint32le(buffer);
            if(total != NULL) *total = vorbiscomment->tot;
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_comment_length(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, uint32_t* length) {
    uint8_t buffer[4];
    MINIFLAC_RESULT r = MINIFLAC_ERROR;

    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: /* fall-through */
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING: /* fall-through */
        case MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS: {
            r = miniflac_vorbiscomment_total_comments(vorbiscomment,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH: {
            case_miniflac_vorbiscomment_comment_length:
            if(vorbiscomment->cur == vorbiscomment->tot) {
                return MINIFLAC_ITERATOR_END;
            }

            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            vorbiscomment->len = miniflac_unpack_uint32le(buffer);
            vorbiscomment->pos = 0;
            if(length != NULL) *length = vorbiscomment->len;
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_COMMENT_STRING;
            return MINIFLAC_OK;
        }
        case MINIFLAC_VORBISCOMMENT_COMMENT_STRING: {
            r = miniflac_vorbiscomment_comment_string(vorbiscomment,br,NULL,0,NULL);
            if(r != MINIFLAC_OK) return r;
            goto case_miniflac_vorbiscomment_comment_length;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbiscomment_comment_string(miniflac_vorbiscomment_t* vorbiscomment, miniflac_bitreader *br, char* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    char c;

    switch(vorbiscomment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH:
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING:
        case MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS:
        case MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH: {
            r = miniflac_vorbiscomment_comment_length(vorbiscomment,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_COMMENT_STRING: {
            while(vorbiscomment->pos < vorbiscomment->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && vorbiscomment->pos < length) {
                    output[vorbiscomment->pos] = c;
                }
                vorbiscomment->pos++;
            }
            if(outlen != NULL) {
                *outlen = vorbiscomment->len <= length ? vorbiscomment->len : length;
            }
            vorbiscomment->cur++;
            vorbiscomment->state = MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}
