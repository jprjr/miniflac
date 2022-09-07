#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

/*
 * utility to take an existing FLAC file but remove all metadata headers,
 * which is technically an invalid stream but useful for testing
 */


int main(int argc, const char* argv[]) {
    int r = 1;
    FILE *input = NULL;
    FILE *output = NULL;
    unsigned char *buffer = NULL;
    long int length = 0;
    size_t pos = 0;

    if(argc < 3) {
        fprintf(stderr,"Usage: %s /path/to/flac /path/to/output\n",argv[0]);
        goto cleanup;
    }

    input = fopen(argv[1],"rb");
    if(input == NULL) {
        fprintf(stderr,"Unable to open %s for reading: %s\n",
          argv[1],
          strerror(errno));
        goto cleanup;
    }

    output = fopen(argv[2],"wb");
    if(output == NULL) {
        fprintf(stderr,"Unable to open %s for writing: %s\n",
        argv[2],
        strerror(errno));
        goto cleanup;
    }

    if(fseek(input,0,SEEK_END)) {
        fprintf(stderr,"Unable to seek input: %s\n",strerror(errno));
        goto cleanup;
    }

    if( (length = ftell(input)) < 0) {
        fprintf(stderr,"Unable to get file length: %s\n",strerror(errno));
        goto cleanup;
    }

    if(fseek(input,0,SEEK_SET)) {
        fprintf(stderr,"Unable to seek input: %s\n",strerror(errno));
        goto cleanup;
    }

    buffer = (uint8_t*)malloc((size_t)length);
    if(buffer == NULL) {
        fprintf(stderr,"Unable to allocate buffer\n");
        goto cleanup;
    }

    if(fread(buffer,1,(size_t)length,input) != (size_t)length) {
        fprintf(stderr,"Unable to read file\n");
        goto cleanup;
    }

    if((char)buffer[pos++] != 'f' ||
       (char)buffer[pos++] != 'L' ||
       (char)buffer[pos++] != 'a' ||
       (char)buffer[pos++] != 'C') {
        fprintf(stderr,"Unable to find fLaC marker\n");
        goto cleanup;
    }

    while(buffer[pos] != 0xFF) {
        uint8_t last_flag = buffer[pos] & 0x80 >> 7;
        uint8_t block_type = buffer[pos++] & 0x7F;
        uint32_t block_length = ((uint32_t)buffer[pos++]) << 16;
        block_length |= ((uint32_t)buffer[pos++]) << 8;
        block_length |= ((uint32_t)buffer[pos++]);
        fprintf(stderr,"last: %u\n",last_flag);
        fprintf(stderr,"block: %u\n",block_type);
        fprintf(stderr,"length: %u\n",block_length);
        pos += block_length;
    }

    if(fwrite(&buffer[pos],1,length - pos,output) != length - pos) {
        fprintf(stderr,"Unable to write file: %s\n", strerror(errno));
        goto cleanup;
    }

    r = 0;
    cleanup:
    if(buffer != NULL) free(buffer);
    if(input != NULL) fclose(input);
    if(output != NULL) fclose(output);
    return r;
}

