#include "padding.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_padding_init(miniflac_padding_t* padding) {
    padding->len = 0;
    padding->pos = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_padding_read_length(miniflac_padding_t* padding, miniflac_bitreader_t* br, uint32_t* length) {
    (void)padding;
    (void)br;
    if(length != NULL) {
        *length = padding->len;
    }
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_padding_read_data(miniflac_padding_t* padding, miniflac_bitreader_t* br, uint8_t* output, uint32_t length, uint32_t* outlen) {
    uint8_t d;
    while(padding->pos < padding->len) {
        if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
        d = (uint8_t) miniflac_bitreader_read(br,8);
        if(output != NULL && padding->pos < length) {
            output[padding->pos] = d;
        }
        padding->pos++;
    }
    if(outlen != NULL) {
        *outlen = padding->len <= length ? padding->len : length;
    }
    return MINIFLAC_OK;
}

