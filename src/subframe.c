/* SPDX-License-Identifier: 0BSD */
#include "subframe.h"

MINIFLAC_PRIVATE
void
miniflac_subframe_init(miniflac_subframe_t* subframe) {
    subframe->bps = 0;
    subframe->state = MINIFLAC_SUBFRAME_HEADER;
    miniflac_subframe_header_init(&subframe->header);
    miniflac_subframe_constant_init(&subframe->constant);
    miniflac_subframe_verbatim_init(&subframe->verbatim);
    miniflac_subframe_fixed_init(&subframe->fixed);
    miniflac_subframe_lpc_init(&subframe->lpc);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_decode(miniflac_subframe_t* subframe, miniflac_bitreader_t* br, int32_t* output, uint32_t block_size, uint8_t bps) {
    MINIFLAC_RESULT r;
    uint32_t i;

    switch(subframe->state) {
        case MINIFLAC_SUBFRAME_HEADER: {
            r = miniflac_subframe_header_decode(&subframe->header,br);
            if(r != MINIFLAC_OK) return r;

            subframe->bps = bps - subframe->header.wasted_bits;

            switch(subframe->header.type) {
                case MINIFLAC_SUBFRAME_TYPE_CONSTANT: {
                    subframe->state = MINIFLAC_SUBFRAME_CONSTANT;
                    goto miniflac_subframe_constant;
                }
                case MINIFLAC_SUBFRAME_TYPE_VERBATIM: {
                    subframe->state = MINIFLAC_SUBFRAME_VERBATIM;
                    goto miniflac_subframe_verbatim;
                }
                case MINIFLAC_SUBFRAME_TYPE_FIXED: {
                    subframe->state = MINIFLAC_SUBFRAME_FIXED;
                    goto miniflac_subframe_fixed;
                }
                case MINIFLAC_SUBFRAME_TYPE_LPC: {
                    subframe->state = MINIFLAC_SUBFRAME_LPC;
                    goto miniflac_subframe_lpc;
                }
                default: {
                    miniflac_abort();
                    return MINIFLAC_ERROR;
                }
            }
            break;
        }

        case MINIFLAC_SUBFRAME_CONSTANT: {
            miniflac_subframe_constant:
            r = miniflac_subframe_constant_decode(&subframe->constant,br,output,block_size,subframe->bps);
            if(r != MINIFLAC_OK) return r;
            break;
        }
        case MINIFLAC_SUBFRAME_VERBATIM: {
            miniflac_subframe_verbatim:
            r = miniflac_subframe_verbatim_decode(&subframe->verbatim,br,output,block_size,subframe->bps);
            if(r != MINIFLAC_OK) return r;
            break;
        }
        case MINIFLAC_SUBFRAME_FIXED: {
            miniflac_subframe_fixed:
            r = miniflac_subframe_fixed_decode(&subframe->fixed,br,output,block_size,subframe->bps,subframe->header.order);
            if(r != MINIFLAC_OK) return r;
            break;
        }
        case MINIFLAC_SUBFRAME_LPC: {
            miniflac_subframe_lpc:
            r = miniflac_subframe_lpc_decode(&subframe->lpc,br,output,block_size,subframe->bps,subframe->header.order);
            if(r != MINIFLAC_OK) return r;
            break;
        }
        default: break;
    }

    if(output != NULL && subframe->header.wasted_bits > 0) {
        for(i=0;i<block_size;i++) {
            output[i] <<= subframe->header.wasted_bits;
        }
    }

    miniflac_subframe_init(subframe);
    return MINIFLAC_OK;
}

