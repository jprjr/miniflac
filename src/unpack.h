/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_UNPACK_H
#define MINIFLAC_UNPACK_H

#include "common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
uint32_t
miniflac_unpack_uint32le(uint8_t buffer[4]);

MINIFLAC_PRIVATE
int32_t
miniflac_unpack_int32le(uint8_t buffer[4]);

MINIFLAC_PRIVATE
uint64_t
miniflac_unpack_uint64le(uint8_t buffer[8]);

MINIFLAC_PRIVATE
int64_t
miniflac_unpack_int64le(uint8_t buffer[8]);

#ifdef __cplusplus
}
#endif

#endif
