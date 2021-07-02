/* SPDX-License-Identifier: 0BSD */
#include "pack.h"

void
pack_uint32le(uint8_t* output, uint32_t n) {
    output[0] = (uint8_t)(n & 0xFF);
    output[1] = (uint8_t)(n >> 8  );
    output[2] = (uint8_t)(n >> 16 );
    output[3] = (uint8_t)(n >> 24 );
}

void
pack_int32le(uint8_t* output, int32_t n) {
    pack_uint32le(output,(uint32_t)n);
}

void
pack_uint24le(uint8_t* output, uint32_t n) {
    output[0] = (uint8_t)(n & 0xFF);
    output[1] = (uint8_t)(n >> 8  );
    output[2] = (uint8_t)(n >> 16 );
}

void
pack_int24le(uint8_t* output, int32_t n) {
    pack_uint24le(output,(uint32_t)n);
}

void
pack_uint16le(uint8_t* output, uint16_t n) {
    output[0] = (uint8_t)(n & 0xFF);
    output[1] = (uint8_t)(n >> 8  );
}

void
pack_int16le(uint8_t* output, int16_t n) {
    pack_uint16le(output,(uint16_t)n);
}

void uint8_packer(uint8_t *outSamples, int32_t* samples[8], uint32_t channels, uint32_t frame_size) {
    uint32_t i = 0;
    uint32_t j = 0;
    for(i=0;i<frame_size;i++) {
        for(j=0;j<channels;j++) {
            outSamples[1 * ((i*channels) + j)] = (uint8_t)samples[j][i];
        }
    }
}

void int16_packer(uint8_t *outSamples, int32_t* samples[8], uint32_t channels, uint32_t frame_size) {
    uint32_t i = 0;
    uint32_t j = 0;
    for(i=0;i<frame_size;i++) {
        for(j=0;j<channels;j++) {
            pack_int16le(&outSamples[2 * ((i*channels) + j)],samples[j][i]);
        }
    }
}

void int24_packer(uint8_t *outSamples, int32_t* samples[8], uint32_t channels, uint32_t frame_size) {
    uint32_t i = 0;
    uint32_t j = 0;
    for(i=0;i<frame_size;i++) {
        for(j=0;j<channels;j++) {
            pack_int24le(&outSamples[3 * ((i*channels) + j)],samples[j][i]);
        }
    }
}

void int32_packer(uint8_t *outSamples, int32_t* samples[8], uint32_t channels, uint32_t frame_size) {
    uint32_t i = 0;
    uint32_t j = 0;
    for(i=0;i<frame_size;i++) {
        for(j=0;j<channels;j++) {
            pack_int32le(&outSamples[4 * ((i*channels) + j)],samples[j][i]);
        }
    }
}

