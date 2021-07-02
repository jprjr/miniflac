/* SPDX-License-Identifier: 0BSD */

/* this just dumps info about the miniflac struct */
#include "../src/flac.h"
#include "../src/debug.h"

int main(void) {
    miniflac_t decoder;
    miniflac_init(&decoder);
    miniflac_dump_flac(&decoder,0);
    return 0;
}

