/* SPDX-License-Identifier: 0BSD */
#include "subframe_fixed.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_subframe_fixed_init(miniflac_subframe_fixed_t* f) {
    f->pos   = 0;
    f->state = MINIFLAC_SUBFRAME_FIXED_DECODE;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_fixed_decode(miniflac_subframe_fixed_t* f, miniflac_bitreader_t* br, int32_t* output, uint32_t block_size, uint8_t bps, miniflac_residual_t* residual, uint8_t predictor_order) {
    int32_t sample;

    int64_t sample1;
    int64_t sample2;
    int64_t sample3;
    int64_t sample4;
    int64_t current_residual;

    MINIFLAC_RESULT r;
    while(f->pos < predictor_order) {
        if(miniflac_bitreader_fill(br,bps)) return MINIFLAC_CONTINUE;
        sample = (int32_t) miniflac_bitreader_read_signed(br,bps);
        if(output != NULL) {
            output[f->pos] = sample;
        }
        f->pos++;
    }
    r = miniflac_residual_decode(residual,br,&f->pos,block_size,predictor_order,output);
    if(r != MINIFLAC_OK) return r;

    if(output != NULL) {
        switch(predictor_order) {
            case 0:
#if 0
                /* this is here for reference but not actually needed */
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    current_residual = output[f->pos];
                    output[f->pos] = (int32_t)current_residual;
                }
#endif
                break;
            case 1: {
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    current_residual = output[f->pos];
                    sample1  = output[f->pos-1];
                    output[f->pos] = (int32_t)(sample1 + current_residual);
                }
                break;
            }
            case 2: {
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    current_residual = output[f->pos];
                    sample1  = output[f->pos-1];
                    sample2  = output[f->pos-2];

                    sample1 *= 2;

                    output[f->pos] = (int32_t)(sample1 - sample2 + current_residual);
                }
                break;
            }
            case 3: {
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    current_residual = output[f->pos];
                    sample1  = output[f->pos-1];
                    sample2  = output[f->pos-2];
                    sample3  = output[f->pos-3];

                    sample1 *= 3;
                    sample2 *= 3;

                    output[f->pos] = (int32_t)(sample1 - sample2 + sample3 + current_residual);
                }
                break;
            }
            case 4: {
                for(f->pos = predictor_order; f->pos < block_size; f->pos++) {
                    current_residual = output[f->pos];
                    sample1  = output[f->pos-1];
                    sample2  = output[f->pos-2];
                    sample3  = output[f->pos-3];
                    sample4  = output[f->pos-4];

                    sample1 *= 4;
                    sample2 *= 6;
                    sample3 *= 4;

                    output[f->pos] = (int32_t)(sample1  - sample2 + sample3 - sample4 + current_residual);
                }
                break;
            }
            default: break;
        }
    }

    return MINIFLAC_OK;

}


