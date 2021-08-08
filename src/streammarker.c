/* SPDX-License-Identifier: 0BSD */
#include "streammarker.h"

MINIFLAC_PRIVATE
void
miniflac_streammarker_init(miniflac_streammarker_t* streammarker) {
    streammarker->state = MINIFLAC_STREAMMARKER_F;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streammarker_decode(miniflac_streammarker_t* streammarker, miniflac_bitreader_t* br) {
    char t;
    switch(streammarker->state) {
        case MINIFLAC_STREAMMARKER_F: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = (char)miniflac_bitreader_read(br,8);
            if(t != 'f') {
                miniflac_abort();
                return MINIFLAC_STREAMMARKER_INVALID;
            }
            streammarker->state = MINIFLAC_STREAMMARKER_L;
        }
        /* fall-through */
        case MINIFLAC_STREAMMARKER_L: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = (char)miniflac_bitreader_read(br,8);
            if(t != 'L') {
                miniflac_abort();
                return MINIFLAC_STREAMMARKER_INVALID;
            }
            streammarker->state = MINIFLAC_STREAMMARKER_A;
        }
        /* fall-through */
        case MINIFLAC_STREAMMARKER_A: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = (char)miniflac_bitreader_read(br,8);
            if(t != 'a') {
                miniflac_abort();
                return MINIFLAC_STREAMMARKER_INVALID;
            }
            streammarker->state = MINIFLAC_STREAMMARKER_C;
        }
        /* fall-through */
        case MINIFLAC_STREAMMARKER_C: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = (char)miniflac_bitreader_read(br,8);
            if(t != 'C') {
                miniflac_abort();
                return MINIFLAC_STREAMMARKER_INVALID;
            }
            break;
        }
        default: {
            miniflac_abort();
            return MINIFLAC_ERROR;
        }
    }

    return MINIFLAC_OK;
}
