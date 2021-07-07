/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_SLURP_H
#define MINIFLAC_SLURP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* load up a file in one shot */
uint8_t* slurp(const char *filename,uint32_t* length);

#ifdef __cplusplus
}
#endif

#endif
