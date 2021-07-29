#include "vorbiscomment.h"
#include "unpack.h"

#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_vorbis_comment_init(miniflac_vorbis_comment_t* vorbis_comment) {
    vorbis_comment->state = MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH;
    vorbis_comment->len = 0;
    vorbis_comment->pos = 0;
    vorbis_comment->tot = 0;
    vorbis_comment->cur = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_vendor_length(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, uint32_t* length) {
    uint8_t buffer[4];
    switch(vorbis_comment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            vorbis_comment->len = miniflac_unpack_uint32le(buffer);
            if(length != NULL) *length = vorbis_comment->len;
            vorbis_comment->state = MINIFLAC_VORBISCOMMENT_VENDOR_STRING;
            return MINIFLAC_OK;
        } default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_vendor_string(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    char c;

    switch(vorbis_comment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: {
            r = miniflac_vorbis_comment_read_vendor_length(vorbis_comment,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING: {
            while(vorbis_comment->pos < vorbis_comment->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && vorbis_comment->pos < length) {
                    output[vorbis_comment->pos] = c;
                }
                vorbis_comment->pos++;
            }
            if(outlen != NULL) {
                *outlen = vorbis_comment->len <= length ? vorbis_comment->len : length;
            }
            vorbis_comment->state = MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_total(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, uint32_t* total) {
    uint8_t buffer[4];
    MINIFLAC_RESULT r = MINIFLAC_ERROR;

    switch(vorbis_comment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: /* fall-through */
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING: {
            r = miniflac_vorbis_comment_read_vendor_string(vorbis_comment,br,NULL,0,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            vorbis_comment->tot = miniflac_unpack_uint32le(buffer);
            if(total != NULL) *total = vorbis_comment->tot;
            vorbis_comment->state = MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_length(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, uint32_t* length) {
    uint8_t buffer[4];
    MINIFLAC_RESULT r = MINIFLAC_ERROR;

    switch(vorbis_comment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH: /* fall-through */
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING: /* fall-through */
        case MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS: {
            r = miniflac_vorbis_comment_read_total(vorbis_comment,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH: {
            case_miniflac_vorbis_comment_comment_length:
            if(vorbis_comment->cur == vorbis_comment->tot) {
                return MINIFLAC_METADATA_END;
            }

            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            vorbis_comment->len = miniflac_unpack_uint32le(buffer);
            vorbis_comment->pos = 0;
            if(length != NULL) *length = vorbis_comment->len;
            vorbis_comment->state = MINIFLAC_VORBISCOMMENT_COMMENT_STRING;
            return MINIFLAC_OK;
        }
        case MINIFLAC_VORBISCOMMENT_COMMENT_STRING: {
            r = miniflac_vorbis_comment_read_string(vorbis_comment,br,NULL,0,NULL);
            if(r != MINIFLAC_OK) return r;
            goto case_miniflac_vorbis_comment_comment_length;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_vorbis_comment_read_string(miniflac_vorbis_comment_t* vorbis_comment, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    char c;

    switch(vorbis_comment->state) {
        case MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH:
        case MINIFLAC_VORBISCOMMENT_VENDOR_STRING:
        case MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS:
        case MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH: {
            r = miniflac_vorbis_comment_read_length(vorbis_comment,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_VORBISCOMMENT_COMMENT_STRING: {
            while(vorbis_comment->pos < vorbis_comment->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && vorbis_comment->pos < length) {
                    output[vorbis_comment->pos] = c;
                }
                vorbis_comment->pos++;
            }
            if(outlen != NULL) {
                *outlen = vorbis_comment->len <= length ? vorbis_comment->len : length;
            }
            vorbis_comment->cur++;
            vorbis_comment->state = MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}
