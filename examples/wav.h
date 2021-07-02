/* SPDX-License-Identifier: 0BSD */
#include <stdio.h>
#include <stdint.h>

int wav_header_create(FILE* output, uint32_t sample_rate, uint32_t chnnels, uint32_t bit_depth);
int wav_header_finish(FILE* output, uint32_t bit_depth);
