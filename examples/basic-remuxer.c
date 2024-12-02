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

/* example program that takes FLAC frames and re-muxes into
 * a new FLAC stream - demonstrating the use of using miniflac
 * to demux FLAC frames */

struct membuffer_s {
    uint8_t* buffer;
    uint32_t pos;
    uint32_t len;
};

typedef struct membuffer_s membuffer_t;


/* this programs reads the entire FLAC file into memory, so if any function
 * returns something besides MINIFLAC_OK, we have an issue, so just abort() */

int main(int argc, const char *argv[]) {
    MINIFLAC_RESULT res;
    int r = 1;
    FILE* output = NULL;
    uint32_t used = 0;
    uint32_t frameTotal = 0;
    miniflac_t* decoder = NULL;

    membuffer_t mem;

    if(argc < 3) {
        fprintf(stderr,"Usage: %s /path/to/flac /path/to/flac\n",argv[0]);
        goto cleanup;
    }

    fprintf(stderr,"Built with miniflac version %s\n",miniflac_version_string());

    mem.buffer = slurp(argv[1],&mem.len);
    if(mem.buffer == NULL) goto cleanup;
    mem.pos = 0;

    output = fopen(argv[2],"wb");
    if(output == NULL) {
        fprintf(stderr,"Failed to open %s: %s\n",argv[2],strerror(errno));
        goto cleanup;
    }

    decoder = (miniflac_t*)malloc(miniflac_size());
    if(decoder == NULL) {
        fprintf(stderr,"Failed to allocate decoder\n");
        goto cleanup;
    }

    miniflac_init(decoder,MINIFLAC_CONTAINER_UNKNOWN);

    if(miniflac_sync(decoder,&mem.buffer[mem.pos],mem.len,&used) != MINIFLAC_OK) abort();
    mem.len -= used;
    mem.pos += used;

    fwrite("fLaC",1,4,output);

    /* work our way through the metadata frames */
    while(decoder->state == MINIFLAC_METADATA) {
        printf("metadata block: type: %u, is_last: %u, length: %u\n",
          decoder->metadata.header.type_raw,
          decoder->metadata.header.is_last,
          decoder->metadata.header.length);
        fflush(stdout);

        if(decoder->metadata.header.type == MINIFLAC_METADATA_STREAMINFO) {
          /* write out the streaminfo block */
          mem.buffer[mem.pos - 4] = 0x80; /* overwrite as last-metadata STREAMINFO */
          fwrite(&mem.buffer[mem.pos - 4],1,4,output);
          fwrite(&mem.buffer[mem.pos],1,decoder->metadata.header.length,output);
        }

        if(miniflac_sync(decoder,&mem.buffer[mem.pos],mem.len,&used) != MINIFLAC_OK) abort();
        mem.len -= used;
        mem.pos += used;
    }

    /* now we're at the beginning of a frame (just past the frame header) and can start demuxing */
    /* by passing NULL - we perform a decode but don't write audio samples */
    while( (res = miniflac_decode(decoder,&mem.buffer[mem.pos],mem.len,&used,NULL)) == MINIFLAC_OK) {
        mem.len -= used;
        mem.pos += used;

        /* write the raw bytes out */

        fwrite(&mem.buffer[mem.pos - decoder->frame.size],1,decoder->frame.size,output);
        frameTotal++;
        if(frameTotal % 10 == 0) {
            fprintf(stderr,"remuxed %u frames\n",frameTotal);
        }

        /* sync up to the next frame boundary */
        res = miniflac_sync(decoder,&mem.buffer[mem.pos],mem.len,&used);
        mem.len -= used;
        mem.pos += used;
        if(res != MINIFLAC_OK) break;
    }

    fprintf(stderr,"decoded %u frames\n",frameTotal);
    fprintf(stderr,"result: %d\n",res);
    if(res < 0) {
        miniflac_dump_flac(decoder,0);
    }

    assert(res >= 0);
    r = 0;

    cleanup:
    if(output != NULL) fclose(output);
    if(mem.buffer != NULL) free(mem.buffer);
    if(decoder != NULL) free(decoder);

    return r;
}
