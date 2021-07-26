/* SPDX-License-Identifier: 0BSD */
#include "subframeheader.h"

MINIFLAC_PRIVATE
void
miniflac_subframe_header_init(miniflac_subframe_header* subframeheader) {
    subframeheader->state       = MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1;
    subframeheader->type        = MINIFLAC_SUBFRAME_TYPE_UNKNOWN;
    subframeheader->order       = 0;
    subframeheader->wasted_bits = 0;
    subframeheader->type_raw    = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_header_decode(miniflac_subframe_header* subframeheader, miniflac_bitreader_t* br) {
    uint64_t t = 0;
    switch(subframeheader->state) {
        case MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1: {
            if(miniflac_bitreader_fill(br,1)) {
                break;
            }
            t = miniflac_bitreader_read(br,1);
            if(t != 0) {
                miniflac_abort();
                return MINIFLAC_SUBFRAME_RESERVED_BIT;
            }
            subframeheader->state = MINIFLAC_SUBFRAME_HEADER_KIND;
        }
        /* fall-through */
        case MINIFLAC_SUBFRAME_HEADER_KIND: {
            if(miniflac_bitreader_fill(br,6)) {
                break;
            }
            t = (uint8_t)miniflac_bitreader_read(br,6);
            subframeheader->type_raw = t;
            if(t == 0) {
                subframeheader->type = MINIFLAC_SUBFRAME_TYPE_CONSTANT;
            } else if(t == 1) {
                subframeheader->type = MINIFLAC_SUBFRAME_TYPE_VERBATIM;
            } else if(t < 8) {
                miniflac_abort();
                return MINIFLAC_SUBFRAME_RESERVED_TYPE;
            } else if(t < 13) {
                subframeheader->type = MINIFLAC_SUBFRAME_TYPE_FIXED;
                subframeheader->order = t - 8;
            } else if(t < 32) {
                miniflac_abort();
                return MINIFLAC_SUBFRAME_RESERVED_TYPE;
            } else {
                subframeheader->type = MINIFLAC_SUBFRAME_TYPE_LPC;
                subframeheader->order = t - 31;
            }
            subframeheader->state = MINIFLAC_SUBFRAME_HEADER_WASTED_BITS;
        }
        /* fall-through */
        case MINIFLAC_SUBFRAME_HEADER_WASTED_BITS: {
            if(miniflac_bitreader_fill(br,1)) {
                break;
            }
            subframeheader->wasted_bits = 0;

            t = miniflac_bitreader_read(br,1);
            if(t == 0) { /* no wasted bits, we're done */
                subframeheader->state = MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1;
                return MINIFLAC_OK;
            }
            subframeheader->state = MINIFLAC_SUBFRAME_HEADER_UNARY;
        }
        /* fall-through */
        case MINIFLAC_SUBFRAME_HEADER_UNARY: {
            while(!miniflac_bitreader_fill(br,1)) {
                subframeheader->wasted_bits++;
                t = miniflac_bitreader_read(br,1);

                if(t == 1) {
                    /* no more wasted bits, we're done */
                    subframeheader->state = MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1;
                    return MINIFLAC_OK;
                }
            }
        }
        /* fall-through */
        default: break;
    }
    return MINIFLAC_CONTINUE;
}
