/* SPDX-License-Identifier: 0BSD */
#include "subframe_constant.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_subframe_constant_init(miniflac_subframe_constant_t* c) {
    c->state = MINIFLAC_SUBFRAME_CONSTANT_DECODE;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_constant_decode(miniflac_subframe_constant_t* c, miniflac_bitreader_t* br, int32_t* output, uint32_t block_size, uint8_t bps) {
    int32_t sample;
    uint32_t i;
    (void)c;

    if(miniflac_bitreader_fill(br,bps)) return MINIFLAC_CONTINUE;
    sample = (int32_t) miniflac_bitreader_read_signed(br,bps);

    if(output != NULL) {
        for(i=0;i<block_size;i++) {
            output[i] = sample;
        }
    }

    return MINIFLAC_OK;
}
