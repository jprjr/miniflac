/* SPDX-License-Identifier: 0BSD */
#define MINIFLAC_IMPLEMENTATION
#include "../miniflac.h"
#include "../src/debug.h"
#include "wav.h"
#include "pack.h"
#include "slurp.h"

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
    FILE* output = NULL;
    uint32_t length = 0;
    uint32_t used = 0;
    uint32_t pos = 0;
    uint32_t frameTotal = 0;
    miniflac_t* decoder = NULL;
    unsigned char* buffer = NULL;
    int32_t** samples = NULL;
    uint8_t* outSamples = NULL;

    uint32_t incoming_string_length = 0;
    char* string_buffer = NULL;
    uint32_t string_buffer_length = 0;

    if(argc < 3) {
        fprintf(stderr,"Usage: %s /path/to/flac /path/to/pcm\n",argv[0]);
        goto cleanup;
    }

    buffer = slurp(argv[1],&length);
    if(buffer == NULL) goto cleanup;

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

    miniflac_init(decoder,MINIFLAC_CONTAINER_UNKNOWN);

    if(miniflac_sync(decoder,&buffer[pos],length,&used) != MINIFLAC_OK) abort();
    length -= used;
    pos += used;

    /* work our way through the metadata frames */
    while(decoder->state == MINIFLAC_METADATA) {
        if(decoder->metadata.header.type == MINIFLAC_METADATA_VORBIS_COMMENT) {
            uint32_t i = 0;
            if(miniflac_vendor_length(decoder,&buffer[pos],length,&used,&incoming_string_length) != MINIFLAC_OK) abort();
            length -= used;
            pos += used;
            if(incoming_string_length > string_buffer_length) {
                string_buffer = realloc(string_buffer,incoming_string_length+1);
                if(string_buffer == NULL) abort();
                string_buffer_length = incoming_string_length;
            }
            fprintf(stdout,"[vendor string][%u]: ",incoming_string_length);
            if(miniflac_vendor_string(decoder,&buffer[pos],length,&used,string_buffer,string_buffer_length+1,&incoming_string_length) != MINIFLAC_OK)
                abort();
            length -= used;
            pos += used;
            string_buffer[incoming_string_length] = '\0';
            fprintf(stdout,"%s\n",string_buffer);

            while( (res =miniflac_comment_length(decoder,&buffer[pos],length,&used,&incoming_string_length)) == MINIFLAC_OK) {
                length -= used;
                pos += used;
                fprintf(stdout,"[comment %u][%u]:\n",i++,incoming_string_length);
                if(incoming_string_length > string_buffer_length) {
                    string_buffer = realloc(string_buffer,incoming_string_length+1);
                    if(string_buffer == NULL) abort();
                    string_buffer_length = incoming_string_length;
                }
                if(miniflac_comment_string(decoder,&buffer[pos],length,&used,string_buffer,string_buffer_length+1,&incoming_string_length) != MINIFLAC_OK)
                    abort();
                length -= used;
                pos += used;
                string_buffer[incoming_string_length] = '\0';
                fprintf(stdout,"%s\n",string_buffer);
            }

            length -= used;
            pos += used;

            if(res != MINIFLAC_ITERATOR_END) abort();
        }

        if(miniflac_sync(decoder,&buffer[pos],length,&used) != MINIFLAC_OK) abort();
        length -= used;
        pos += used;
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
    if(string_buffer != NULL) free(string_buffer);

    return r;
}
