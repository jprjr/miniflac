/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_APPLICATION_H
#define MINIFLAC_APPLICATION_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

enum MINIFLAC_APPLICATION_STATE {
    MINIFLAC_APPLICATION_ID,
    MINIFLAC_APPLICATION_DATA,
};

struct miniflac_application_s {
    enum MINIFLAC_APPLICATION_STATE state;
    uint32_t len; /* length of data */
    uint32_t pos; /* current byte */
};

typedef struct miniflac_application_s miniflac_application_t;
typedef enum MINIFLAC_APPLICATION_STATE MINIFLAC_APPLICATION_STATE;

/* note: len is set outside of application block functions */

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_application_init(miniflac_application_t* application);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_application_read_id(miniflac_application_t* application, miniflac_bitreader_t* br, uint32_t* id);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_application_read_length(miniflac_application_t* application, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_application_read_data(miniflac_application_t* application, miniflac_bitreader_t* br, uint8_t* output, uint32_t length, uint32_t* outlen);

#ifdef __cplusplus
}
#endif

#endif
