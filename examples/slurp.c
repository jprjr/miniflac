#include "slurp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

uint8_t* slurp(const char *filename, uint32_t* length) {
    uint8_t* buffer = NULL;
    long int pos = 0;
    FILE *input = NULL;
    if(filename == NULL) return NULL;
    if(length == NULL) return NULL;

    input = fopen(filename,"rb");
    if(input == NULL) {
        fprintf(stderr,"Unable to open %s: %s\n",
          filename,
          strerror(errno));
        goto slurp_cleanup;
    }

    if(fseek(input,0,SEEK_END)) {
        fprintf(stderr,"Unable to seek to end: %s\n",
          strerror(errno));
        goto slurp_cleanup;
    }

    pos = ftell(input);
    if(pos <= 0) {
        if(pos != 0) {
          fprintf(stderr,"Unable to get length: %s\n",
            strerror(errno));
        }
        goto slurp_cleanup;
    }

    if(fseek(input,0,SEEK_SET)) {
        fprintf(stderr,"Unable to seek to start: %s\n",
          strerror(errno));
        goto slurp_cleanup;
    }

    buffer = (uint8_t*)malloc((size_t)pos);
    if(buffer == NULL) {
        fprintf(stderr,"out of memory\n");
        goto slurp_cleanup;
    }

    if(fread(buffer,1,(size_t)pos,input) != (size_t)pos) {
        free(buffer);
        buffer = NULL;
        fprintf(stderr,"Error reading: %s\n",
          strerror(errno));
        goto slurp_cleanup;
    }

    slurp_cleanup:
    if(input != NULL) fclose(input);
    *length = (uint32_t)pos;
    return buffer;
}

