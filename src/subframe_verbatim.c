/* SPDX-License-Identifier: 0BSD */
#include "subframe_verbatim.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_subframe_verbatim_init(miniflac_subframe_verbatim *c) {
    c->pos   = 0;
    c->state = MINIFLAC_SUBFRAME_VERBATIM_DECODE;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_verbatim_decode(miniflac_subframe_verbatim* c, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps) {
    int32_t sample;

    while(c->pos < block_size) {
        if(miniflac_bitreader_fill(br,bps)) return MINIFLAC_CONTINUE;
        sample = (int32_t) miniflac_bitreader_read_signed(br,bps);
        if(output != NULL) {
            output[c->pos] = sample;
        }
        c->pos++;
    }

    c->pos = 0;
    return MINIFLAC_OK;

}

