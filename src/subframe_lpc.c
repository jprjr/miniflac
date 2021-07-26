/* SPDX-License-Identifier: 0BSD */
#include "subframe_lpc.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_subframe_lpc_init(miniflac_subframe_lpc_t* l) {
    unsigned int i;
    l->pos   = 0;
    l->precision = 0;
    l->shift = 0;
    l->coeff = 0;
    for(i = 0; i < 32; i++) {
        l->coefficients[i] = 0;
    }
    l->state = MINIFLAC_SUBFRAME_LPC_PRECISION;
    miniflac_residual_init(&l->residual);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_lpc_decode(miniflac_subframe_lpc_t* l, miniflac_bitreader_t* br, int32_t* output, uint32_t block_size, uint8_t bps, uint8_t predictor_order) {
    int32_t sample;
    int64_t temp;
    int64_t prediction;
    uint32_t i,j;
    MINIFLAC_RESULT r;

    while(l->pos < predictor_order) {
        if(miniflac_bitreader_fill(br,bps)) return MINIFLAC_CONTINUE;
        sample = (int32_t) miniflac_bitreader_read_signed(br,bps);
        if(output != NULL) {
            output[l->pos] = sample;
        }
        l->pos++;
        l->state = MINIFLAC_SUBFRAME_LPC_PRECISION;
    }

    if(l->state == MINIFLAC_SUBFRAME_LPC_PRECISION) {
        if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
        l->precision = miniflac_bitreader_read(br,4) + 1;
        l->state = MINIFLAC_SUBFRAME_LPC_SHIFT;
    }

    if(l->state == MINIFLAC_SUBFRAME_LPC_SHIFT) {
        if(miniflac_bitreader_fill(br,5)) return MINIFLAC_CONTINUE;
        temp = miniflac_bitreader_read_signed(br,5);
        if(temp < 0) temp = 0;
        l->shift = temp;
        l->state = MINIFLAC_SUBFRAME_LPC_COEFF;
    }

    if(l->state == MINIFLAC_SUBFRAME_LPC_COEFF) {
        while(l->coeff < predictor_order) {
            if(miniflac_bitreader_fill(br,l->precision)) return MINIFLAC_CONTINUE;
            sample = (int32_t) miniflac_bitreader_read_signed(br,l->precision);
            l->coefficients[l->coeff++] = sample;
        }
    }

    r = miniflac_residual_decode(&l->residual,br,&l->pos,block_size,predictor_order,output);
    if(r != MINIFLAC_OK) return r;

    if(output != NULL) {
        for(i=predictor_order;i<block_size;i++) {
            prediction = 0;
            for(j=0;j<predictor_order;j++) {
                temp = output[i - j - 1];
                temp *= l->coefficients[j];
                prediction += temp;
            }
            prediction >>= l->shift;
            prediction += output[i];
            output[i] = prediction;
        }
    }

    miniflac_subframe_lpc_init(l);
    return MINIFLAC_OK;
}



