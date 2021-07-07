/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_OGGHEADER_H
#define MINIFLAC_OGGHEADER_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_oggheader_s miniflac_oggheader_t;
typedef enum MINIFLAC_OGGHEADER_STATE MINIFLAC_OGGHEADER_STATE;

enum MINIFLAC_OGGHEADER_STATE {
    MINIFLAC_OGGHEADER_PACKETTYPE,
    MINIFLAC_OGGHEADER_F,
    MINIFLAC_OGGHEADER_L,
    MINIFLAC_OGGHEADER_A,
    MINIFLAC_OGGHEADER_C,
    MINIFLAC_OGGHEADER_MAJOR,
    MINIFLAC_OGGHEADER_MINOR,
    MINIFLAC_OGGHEADER_HEADERPACKETS,
};

struct miniflac_oggheader_s {
    MINIFLAC_OGGHEADER_STATE state;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_oggheader_init(miniflac_oggheader_t* oggheader);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_oggheader_decode(miniflac_oggheader_t* oggheader, miniflac_bitreader* br);

#ifdef __cplusplus
}
#endif

#endif



