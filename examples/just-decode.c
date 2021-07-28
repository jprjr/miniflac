/* SPDX-License-Identifier: 0BSD */
#include "../src/flac.h"
#include "slurp.h"
#include "tictoc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char* argv[]) {
    MINIFLAC_RESULT res;
    miniflac_t decoder;
    TicTocTimer t;

    uint32_t i = 0;
    uint8_t* flac_data = NULL;
    uint32_t flac_len = 0;
    uint32_t pos = 0;
    uint32_t used = 0;
    uint32_t length = 0;
    float elapsed = 0.0f;
    int32_t** samples = NULL;

    if(argc < 2) {
        fprintf(stderr,"Usage: %s /path/to/flac\n",argv[0]);
        return 1;
    }

    flac_data = slurp(argv[1],&flac_len);
    if(flac_data == NULL) return 1;

    samples = (int32_t**)malloc(sizeof(int32_t*) * 8);
    if(samples == NULL) return 1;
    for(i=0;i<8;i++) {
        samples[i] = (int32_t*)malloc(sizeof(int32_t) * 65535);
        if(samples[i] == NULL) return 1;
    }

    length = flac_len;

    t = tic();
    miniflac_init(&decoder, MINIFLAC_CONTAINER_UNKNOWN);
    while( (res = miniflac_decode(&decoder,&flac_data[pos],length,&used,samples)) == MINIFLAC_OK) {
        length -= used;
        pos += used;
    }
    elapsed = toc(&t);

    fprintf(stderr,"elapsed time: %f seconds\n",elapsed);

    free(flac_data);
    for(i=0;i<8;i++) {
        free(samples[i]);
    }
    free(samples);
    return 0;
}
