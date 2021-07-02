/* SPDX-License-Identifier: 0BSD */
#define MINIFLAC_IMPLEMENTATION
#include "../miniflac.h"
#include "../src/debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>

/* just decodes audio and throws it away */

int main(int argc, const char *argv[]) {
    MINIFLAC_RESULT res;
    int r = 1;
    FILE* input = NULL;
    long int temp = 0;
    uint32_t length = 0;
    uint32_t used = 0;
    uint32_t pos = 0;
    uint32_t frameTotal = 0;
    miniflac_t* decoder = NULL;
    unsigned char* buffer = NULL;

    if(argc < 2) {
        fprintf(stderr,"Usage: %s /path/to/flac\n",argv[0]);
        goto cleanup;
    }

    input = fopen(argv[1],"rb");
    if(input == NULL) {
        fprintf(stderr,"Failed to open %s: %s\n",argv[1],strerror(errno));
        goto cleanup;
    }

    decoder = malloc(miniflac_size());
    if(decoder == NULL) {
        fprintf(stderr,"Failed to allocate decoder\n");
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
    while( (res = miniflac_decode(decoder,&buffer[pos],length,&used,NULL)) == MINIFLAC_OK ) {
        length -= used;
        pos += used;
        frameTotal++;
    }

    fprintf(stderr,"decoded %u frames\n",frameTotal);
    fprintf(stderr,"result: %d\n",res);
    if(res < 0) {
        miniflac_dump_flac(decoder,0);
    }

    assert(res >= 0);
    assert(used == 0);
    r = 0;

    cleanup:
    if(input != NULL) fclose(input);
    if(buffer != NULL) free(buffer);
    if(decoder != NULL) free(decoder);

    return r;
}
