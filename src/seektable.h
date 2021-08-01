/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_SEEKTABLE_H
#define MINIFLAC_SEEKTABLE_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_seektable_s miniflac_seektable_t;
typedef enum MINIFLAC_SEEKTABLE_STATE MINIFLAC_SEEKTABLE_STATE;

enum MINIFLAC_SEEKTABLE_STATE {
    MINIFLAC_SEEKTABLE_SAMPLE_NUMBER,
    MINIFLAC_SEEKTABLE_SAMPLE_OFFSET,
    MINIFLAC_SEEKTABLE_SAMPLES,
};

struct miniflac_seektable_s {
    MINIFLAC_SEEKTABLE_STATE    state;
    uint32_t len; /* number of seekpoints */
    uint32_t pos; /* current seekpoint */
};

/* note: len is set outside of seektable functions */

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_seektable_init(miniflac_seektable_t* seektable);

/* read the number of seekpoints */
MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_seektable_read_seekpoints(miniflac_seektable_t* seektable, miniflac_bitreader_t* br, uint32_t* seekpoints);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_seektable_read_sample_number(miniflac_seektable_t* seektable, miniflac_bitreader_t* br, uint64_t* sample_number);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_seektable_read_sample_offset(miniflac_seektable_t* seektable, miniflac_bitreader_t* br, uint64_t* sample_offset);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_seektable_read_samples(miniflac_seektable_t* seektable, miniflac_bitreader_t* br, uint16_t* samples);

#ifdef __cplusplus
}
#endif

#endif
