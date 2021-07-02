/* SPDX-License-Identifier: 0BSD */
#include <stdint.h>

void pack_uint32le(uint8_t* output, uint32_t n);
void pack_int32le(uint8_t* output, int32_t n);

void pack_uint24le(uint8_t* output, uint32_t n);
void pack_int24le(uint8_t* output, int32_t n);

void pack_uint16le(uint8_t* output, uint16_t n);
void pack_int16le(uint8_t* output, int16_t n);

typedef void (*packer)(uint8_t* b, int32_t* pcm[8], uint32_t channels, uint32_t frame_size);

void uint8_packer(uint8_t *outSamples, int32_t* samples[8], uint32_t channels, uint32_t frame_size);
void int16_packer(uint8_t *outSamples, int32_t* samples[8], uint32_t channels, uint32_t frame_size);
void int24_packer(uint8_t *outSamples, int32_t* samples[8], uint32_t channels, uint32_t frame_size);
void int32_packer(uint8_t *outSamples, int32_t* samples[8], uint32_t channels, uint32_t frame_size);
