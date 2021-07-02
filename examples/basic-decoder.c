/* SPDX-License-Identifier: 0BSD */
#define MINIFLAC_IMPLEMENTATION
#include "../miniflac.h"
#include "../src/debug.h"
#include "wav.h"
#include "pack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>

int main(int argc, const char *argv[]) {
    MINIFLAC_RESULT res;
    int r = 1;
    unsigned int i = 0;
    FILE* input = NULL;
    FILE* output = NULL;
    long int temp = 0;
    uint32_t length = 0;
    uint32_t used = 0;
    uint32_t pos = 0;
    uint32_t frameTotal = 0;
    miniflac_t* decoder = NULL;
    unsigned char* buffer = NULL;
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

    decoder = malloc(miniflac_size());
    if(decoder == NULL) {
        fprintf(stderr,"Failed to allocate decoder\n");
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

    if(fseek(input,0,SEEK_END)) {
        fprintf(stderr,"Unable to seek input: %s\n",strerror(errno));
        goto cleanup;
    }

    if( (temp = ftell(input)) < 0) {
        fprintf(stderr,"Invalid file length: %s\n",strerror(errno));
        goto cleanup;
    } else if(temp == 0) {
        fprintf(stderr,"Refusing to read 0-byte file\n");
        goto cleanup;
    }

    if(fseek(input,0,SEEK_SET)) {
        fprintf(stderr,"Unable to seek input: %s\n",strerror(errno));
        goto cleanup;
    }

    length = (size_t)temp;

    buffer = malloc(length);
    if(buffer == NULL) {
        fprintf(stderr,"Unable to allocate buffer\n");
        goto cleanup;
    }

    if(fread(buffer,1,length,input) != length) {
        fprintf(stderr,"Unable to read file\n");
        goto cleanup;
    }

    miniflac_init(decoder);

    /* work our way through the metadata frames */
    while(decoder->state != MINIFLAC_FRAME) {
        res = miniflac_sync(decoder,&buffer[pos],length,&used);
        length -= used;
        pos += used;

        switch(res) {
            case MINIFLAC_OK: break;
            case MINIFLAC_CONTINUE: break;
            default: abort();
        }
    }

    wav_header_create(output,decoder->frame.header.sample_rate,decoder->frame.header.channels,decoder->frame.header.bps);

    /* now we're at the beginning of a frame (just past the frame header) and can start decoding */
    while( (res = miniflac_decode(decoder,&buffer[pos],length,&used,samples)) == MINIFLAC_OK) {
        length -= used;
        pos += used;

        uint32_t len = 0;
        uint32_t sampSize = 0;
        packer pack = NULL;

        switch(decoder->frame.header.bps) {
            case 8:  sampSize = 1; pack = uint8_packer; break;
            case 16: sampSize = 2; pack = int16_packer; break;
            case 24: sampSize = 3; pack = int24_packer; break;
            case 32: sampSize = 4; pack = int32_packer; break;
            default: abort();
        }

        len = sampSize * decoder->frame.header.channels * decoder->frame.header.block_size;

        /* samples is planar, convert into an interleaved format, and pack into little-endian */
        pack(outSamples,samples,decoder->frame.header.channels,decoder->frame.header.block_size);
        fwrite(outSamples,1,len,output);
        frameTotal++;

        /* sync up to the next frame boundary */
        res = miniflac_sync(decoder,&buffer[pos],length,&used);
        length -= used;
        pos += used;
        if(res != MINIFLAC_OK) break;
    }

    fprintf(stderr,"decoded %u frames\n",frameTotal);
    fprintf(stderr,"result: %d\n",res);
    if(res < 0) {
        miniflac_dump_flac(decoder,0);
    }

    assert(res >= 0);
    assert(used == 0);
    wav_header_finish(output,decoder->frame.header.bps);
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
    if(buffer != NULL) free(buffer);
    if(decoder != NULL) free(decoder);

    return r;
}
