/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_H
#define MINIFLAC_H

#include <stdint.h>
#include <stddef.h>

#include "common.h"
#include "ogg.h"
#include "oggheader.h"
#include "bitreader.h"
#include "streammarker.h"
#include "metadata.h"
#include "frame.h"

typedef struct miniflac_s miniflac_t;
typedef enum MINIFLAC_STATE MINIFLAC_STATE;
typedef enum MINIFLAC_CONTAINER MINIFLAC_CONTAINER;

enum MINIFLAC_STATE {
    MINIFLAC_OGGHEADER, /* will try to find an ogg header */
    MINIFLAC_STREAMMARKER_OR_FRAME, /* poke for a stream marker or audio frame 
    */
    MINIFLAC_STREAMMARKER, /* reading a stream marker */
    MINIFLAC_METADATA_OR_FRAME, /* will try to find a frame sync code or try to parse a metadata block */
    MINIFLAC_METADATA, /* currently reading a metadata block */
    MINIFLAC_FRAME,    /* currently reading an audio frame */
};

enum MINIFLAC_CONTAINER {
    MINIFLAC_CONTAINER_UNKNOWN,
    MINIFLAC_CONTAINER_NATIVE,
    MINIFLAC_CONTAINER_OGG,
};

struct miniflac_s {
    MINIFLAC_STATE state;
    MINIFLAC_CONTAINER container;
    miniflac_bitreader br;
    miniflac_ogg_t ogg;
    miniflac_oggheader_t oggheader;
    miniflac_streammarker_t streammarker;
    miniflac_metadata metadata;
    miniflac_frame frame;
};


#ifdef __cplusplus
extern "C" {
#endif

/* returns the number of bytes needed for the miniflac struct (for malloc, etc) */
MINIFLAC_API
MINIFLAC_PURE
size_t
miniflac_size(void);

/* give the container type if you know the kind of container,
 * otherwise 0 for unknown */
MINIFLAC_API
void
miniflac_init(miniflac_t* pFlac, MINIFLAC_CONTAINER container);

MINIFLAC_API
MINIFLAC_RESULT
miniflac_sync(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length);

MINIFLAC_API
MINIFLAC_RESULT
miniflac_decode(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, int32_t** samples);

#ifdef __cplusplus
}
#endif

#endif
