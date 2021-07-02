/* SPDX-License-Identifier: 0BSD */
#include "wav.h"
#include "pack.h"

#include <string.h>
#include <errno.h>

static const char *
extensible_guid_trailer= "\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71";

/* creates an empty wav header with zeros for all values */
int wav_header_create(FILE* output, uint32_t sample_rate, uint32_t channels, uint32_t bit_depth) {
    uint8_t buffer[4];
    /* RIFF ID */
    if(fwrite("RIFF",1,4,output) != 4) return -1;

    /* chunkSize (tbd) */
    pack_uint32le(buffer,0);
    if(fwrite(buffer,1,4,output) != 4) return 1;

    /* WAVE ID */
    if(fwrite("WAVE",1,4,output) != 4) return -1;

    /* "fmt " subchunk */
    if(fwrite("fmt ",1,4,output) != 4) return -1;

    /* fmt chunkSize */
    if(bit_depth > 16) {
        pack_uint32le(buffer,40);
    } else {
        pack_uint32le(buffer,16);
    }
    if(fwrite(buffer,1,4,output) != 4) return -1;

    /* audioFormat */
    if(bit_depth > 16) {
        pack_uint16le(buffer,0xFFFE);
    } else {
        pack_uint16le(buffer,1);
    }
    if(fwrite(buffer,1,2,output) != 2) return -1;

    /* numChannels */
    pack_uint16le(buffer,channels);
    if(fwrite(buffer,1,2,output) != 2) return -1;

    /* sampleRate */
    pack_uint32le(buffer,sample_rate);
    if(fwrite(buffer,1,4,output) != 4) return -1;

    /* data rate (bytes per second) */
    pack_uint32le(buffer,sample_rate * channels * (bit_depth / 8));
    if(fwrite(buffer,1,4,output) != 4) return -1;

    /* block alignment (channels * sample size) */
    pack_uint16le(buffer,channels * (bit_depth / 8));
    if(fwrite(buffer,1,2,output) != 2) return 0;

    /* bits per sample */
    pack_uint16le(buffer,bit_depth);
    if(fwrite(buffer,1,2,output) != 2) return 0;

    if(bit_depth > 16) {
        /* size of extended header */
        pack_uint16le(buffer,22);
        if(fwrite(buffer,1,2,output) != 2) return 0;

        /* number of "valid" bits per sample? */
        pack_uint16le(buffer,bit_depth);
        if(fwrite(buffer,1,2,output) != 2) return 0;

        /* speaker position mask */
        /*
            SPEAKER_FRONT_LEFT	0x1
            SPEAKER_FRONT_RIGHT	0x2
            SPEAKER_FRONT_CENTER	0x4
            SPEAKER_LOW_FREQUENCY	0x8
            SPEAKER_BACK_LEFT	0x10
            SPEAKER_BACK_RIGHT	0x20
            SPEAKER_FRONT_LEFT_OF_CENTER	0x40
            SPEAKER_FRONT_RIGHT_OF_CENTER	0x80
            SPEAKER_BACK_CENTER	0x100
            SPEAKER_SIDE_LEFT	0x200
            SPEAKER_SIDE_RIGHT	0x400
        */
        switch(channels) {
            case 1: pack_uint32le(buffer, 0x04); break;
            case 2: pack_uint32le(buffer, 0x01 | 0x02); break;
            case 3: pack_uint32le(buffer, 0x01 | 0x02 | 0x04); break;
            case 4: pack_uint32le(buffer, 0x01 | 0x02 | 0x200 | 0x400); break;
            case 5: pack_uint32le(buffer, 0x01 | 0x02 | 0x04  | 0x200 | 0x400); break;
            case 6: pack_uint32le(buffer, 0x01 | 0x02 | 0x04  | 0x08  | 0x200  | 0x400); break;
            case 7: pack_uint32le(buffer, 0x01 | 0x02 | 0x04  | 0x08  | 0x100  | 0x200 | 0x400); break;
            case 8: pack_uint32le(buffer, 0x01 | 0x02 | 0x04  | 0x08  | 0x10   | 0x20  |  0x200 | 0x400); break;
            default: pack_uint32le(buffer,0); break;
        }
        if(fwrite(buffer,1,4,output) != 4) return 0;

        /* subformatcode - same as above audioFormat */
        pack_uint16le(buffer,1);
        if(fwrite(buffer,1,2,output) != 2) return 0;

        /* rest of the GUID */
        if(fwrite(extensible_guid_trailer,1,14,output) != 14) return 0;
    }

    /* "data" ID" */
    if(fwrite("data",1,4,output) != 4) return 0;

    /* data size (tbd) */
    pack_uint32le(buffer,0);
    if(fwrite(buffer,1,4,output) != 4) return 0;

    return 0;
}

int wav_header_finish(FILE* output, uint32_t bit_depth) {
    uint8_t buffer[4];
    long int pos = 0;

    if(fseek(output,0,SEEK_END)) {
        return -1;
    }

    pos = ftell(output);
    if(pos == -1) return -1;

    if(fseek(output,4,SEEK_SET)) {
        return -1;
    }
    pack_uint32le(buffer,pos - 8);
    if(fwrite(buffer,1,4,output) != 4) return -1;

    if(bit_depth > 16) {
        if(fseek(output,64,SEEK_SET)) {
            return -1;
        }
        pack_uint32le(buffer,pos - 68);
    } else {
        if(fseek(output,40,SEEK_SET)) {
            return -1;
        }
        pack_uint32le(buffer,pos - 44);
    }

    if(fwrite(buffer,1,4,output) != 4) return 1;
    fseek(output,0,SEEK_END);

    return 0;
}
