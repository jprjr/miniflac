/* SPDX-License-Identifier: 0BSD */
#define MINIFLAC_IMPLEMENTATION
#include "../miniflac.h"
#include "wav.h"
#include "pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>

/* example program that reads a single byte at a time, and doesn't bother
 * using miniflac_sync - it just reads bytes and feeds them into the decoder
 * until it returns MINIFLAC_OK, then writes out the audio */

int main(int argc, const char *argv[]) {
    MINIFLAC_RESULT res;
    int r = 1;
    unsigned int i = 0;
    FILE* input = NULL;
    FILE* output = NULL;
    const uint32_t length = 1;
    uint32_t used = 1;
    uint32_t frameTotal = 0;
    miniflac_t decoder;
    unsigned char buffer[1];
    int32_t** samples = NULL;
    uint8_t* outSamples = NULL;

    if(argc < 3) {
        fprintf(stderr,"Usage: %s /path/to/flac /path/to/pcm\n",argv[0]);
        goto cleanup;
    }

    input = fopen(argv[1],"rb");
    if(input == NULL) {
        fprintf(stderr,"Failed to open %s: %s\n",argv[1],strerror(errno));
        goto cleanup;
    }

    output = fopen(argv[2],"wb");
    if(output == NULL) {
        fprintf(stderr,"Failed to open %s: %s\n",argv[2],strerror(errno));
        goto cleanup;
    }

    samples = (int32_t **)malloc(sizeof(int32_t *) * 8);
    if(samples == NULL) {
        fprintf(stderr,"Failed to allocate sample buffer\n");
        goto cleanup;
    }

    for(i=0;i<8;i++) {
        samples[i] = NULL;
    }

    for(i=0;i<8;i++) {
        samples[i] = (int32_t *)malloc(sizeof(int32_t) * 65535);
        if(samples[i] == NULL) {
            fprintf(stderr,"Failed to allocate channel buffer\n");
            goto cleanup;
        }
    }

    /* allocate max buffer for packed samples */
    outSamples = (uint8_t*)malloc(sizeof(int32_t) * 8 * 65535);
    if(outSamples == NULL) {
        fprintf(stderr,"Failed to allocated pcm buffer\n");
        goto cleanup;
    }

    miniflac_init(&decoder);

    while(fread(buffer,1,1,input)) {
        res = miniflac_decode(&decoder,buffer,length,&used,samples);

        uint32_t len = 0;
        uint32_t sampSize = 0;
        packer pack = 0;

        switch(res) {
            case MINIFLAC_OK: {
                if(ftell(output) == 0) {
                    wav_header_create(output,decoder.frame.header.sample_rate,decoder.frame.header.channels,decoder.frame.header.bps);
                }
                switch(decoder.frame.header.bps) {
                    case 8:  sampSize = 1; pack = uint8_packer; break;
                    case 16: sampSize = 2; pack = int16_packer; break;
                    case 24: sampSize = 3; pack = int24_packer; break;
                    case 32: sampSize = 4; pack = int32_packer; break;
                    default: abort();
                }
                len = sampSize * decoder.frame.header.channels * decoder.frame.header.block_size;
                pack(outSamples,samples,decoder.frame.header.channels,decoder.frame.header.block_size);
                fwrite(outSamples,1,len,output);
                frameTotal++;
                break;
            }
            case MINIFLAC_CONTINUE: break;
            default: {
                fprintf(stderr,"error, decoded %u frames, decoder retruned: %d\n",frameTotal,res);
                fflush(stderr);
                goto cleanup;
            }
        }
    }

    wav_header_finish(output,decoder.frame.header.bps);
    fprintf(stderr,"decoded %u frames\n",frameTotal);
    r = 0;

    cleanup:
    if(input != NULL) fclose(input);
    if(output != NULL) fclose(output);
    if(samples != NULL) {
        for(i=0;i<8;i++) {
            if(samples[i] != NULL) free(samples[i]);
        }
        free(samples);
    }
    if(outSamples != NULL) free(outSamples);

    return r;
}
