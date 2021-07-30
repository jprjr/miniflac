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

/* example program that dumps a bunch of metadata, and decodes audio into
 * a .wav file. */

struct membuffer_s {
    uint8_t* buffer;
    uint32_t pos;
    uint32_t len;
};

typedef struct membuffer_s membuffer_t;


/* this programs reads the entire FLAC file into memory, so if any function
 * returns something besides MINIFLAC_OK, we have an issue, so just abort() */

static void
dump_streaminfo(miniflac_t* decoder, membuffer_t* mem) {
    uint32_t used;
    uint8_t  temp8;
    uint16_t temp16;
    uint32_t temp32;
    uint64_t temp64;
    uint8_t md5[16];

    fprintf(stdout,"[streaminfo]\n");

    if(miniflac_streaminfo_min_block_size(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp16) != MINIFLAC_OK) abort();
    mem->pos += used;
    mem->len -= used;
    fprintf(stdout,"  min_block_size: %u\n",temp16);

    if(miniflac_streaminfo_max_block_size(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp16) != MINIFLAC_OK) abort();
    mem->pos += used;
    mem->len -= used;
    fprintf(stdout,"  max_block_size: %u\n",temp16);

    if(miniflac_streaminfo_min_frame_size(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->pos += used;
    mem->len -= used;
    fprintf(stdout,"  min_frame_size: %u\n",temp16);

    if(miniflac_streaminfo_max_frame_size(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->pos += used;
    mem->len -= used;
    fprintf(stdout,"  max_frame_size: %u\n",temp16);

    if(miniflac_streaminfo_sample_rate(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->pos += used;
    mem->len -= used;
    fprintf(stdout,"  sample_rate: %u\n",temp32);

    if(miniflac_streaminfo_channels(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp8) != MINIFLAC_OK) abort();
    mem->pos += used;
    mem->len -= used;
    fprintf(stdout,"  channels: %u\n",temp8);

    if(miniflac_streaminfo_bps(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp8) != MINIFLAC_OK) abort();
    mem->pos += used;
    mem->len -= used;
    fprintf(stdout,"  bps: %u\n",temp8);

    if(miniflac_streaminfo_total_samples(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp64) != MINIFLAC_OK) abort();
    mem->pos += used;
    mem->len -= used;
    fprintf(stdout,"  total_samples: %lu\n",temp64);

    if(miniflac_streaminfo_md5_data(decoder,&mem->buffer[mem->pos],mem->len,&used,md5,16,NULL) != MINIFLAC_OK) abort();
    mem->pos += used;
    mem->len -= used;
    fprintf(stdout,"  md5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
      md5[ 0], md5[ 1], md5[ 2], md5[ 3],
      md5[ 4], md5[ 5], md5[ 6], md5[ 7],
      md5[ 8], md5[ 9], md5[10], md5[11],
      md5[12], md5[13], md5[14], md5[15]);
    fflush(stdout);

}

static void
dump_vorbis_comment(miniflac_t* decoder, membuffer_t* mem) {
    uint32_t used = 0;
    uint32_t temp32 = 0;
    unsigned int i = 0;
    char* string_buffer = NULL;
    uint32_t string_buffer_len = 0;
    MINIFLAC_RESULT res = MINIFLAC_OK;

    fprintf(stdout,"[vorbis_comment]\n");

    if(miniflac_vorbis_comment_vendor_length(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;

    if(temp32 > string_buffer_len) {
        string_buffer = realloc(string_buffer,temp32+1);
        if(string_buffer == NULL) abort();
        string_buffer_len = temp32;
    }

    fprintf(stdout,"  vendor string=[%u]",temp32);
    if(miniflac_vorbis_comment_vendor_string(decoder,&mem->buffer[mem->pos],mem->len,&used,string_buffer,string_buffer_len+1,&temp32) != MINIFLAC_OK)
        abort();
    mem->len -= used;
    mem->pos += used;
    string_buffer[temp32] = '\0';
    fprintf(stdout,"%s\n",string_buffer);

    while( (res =miniflac_vorbis_comment_length(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32)) == MINIFLAC_OK) {
        mem->len -= used;
        mem->pos += used;
        fprintf(stdout,"  comment[%u]=[%u]",i++,temp32);
        if(temp32 > string_buffer_len) {
            string_buffer = realloc(string_buffer,temp32+1);
            if(string_buffer == NULL) abort();
            string_buffer_len = temp32;
        }
        if(miniflac_vorbis_comment_string(decoder,&mem->buffer[mem->pos],mem->len,&used,string_buffer,string_buffer_len+1,&temp32) != MINIFLAC_OK)
            abort();
        mem->len -= used;
        mem->pos += used;
        string_buffer[temp32] = '\0';
        fprintf(stdout,"%s\n",string_buffer);
    }
    mem->len -= used;
    mem->pos += used;

    if(res != MINIFLAC_METADATA_END) abort();
    free(string_buffer);
    fflush(stdout);
}

static void
dump_picture(miniflac_t* decoder, membuffer_t* mem) {
    uint32_t used = 0;
    uint32_t temp32 = 0;
    char* string_buffer = NULL;
    uint32_t string_buffer_len = 0;

    fprintf(stdout,"[picture]\n");
    if(miniflac_picture_type(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  type=%u\n",temp32);

    if(miniflac_picture_mime_length(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  mime string=[%u]",temp32);
    if(temp32 > string_buffer_len) {
        string_buffer = realloc(string_buffer,temp32 + 1);
        if(string_buffer == NULL) abort();
        string_buffer_len = temp32;
    }
    if(miniflac_picture_mime_string(decoder,&mem->buffer[mem->pos],mem->len,&used,string_buffer,string_buffer_len+1,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    string_buffer[temp32] = '\0';
    fprintf(stdout,"%s\n",string_buffer);

    if(miniflac_picture_description_length(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  description string=[%u]",temp32);
    if(temp32 > string_buffer_len) {
        string_buffer = realloc(string_buffer,temp32 + 1);
        if(string_buffer == NULL) abort();
        string_buffer_len = temp32;
    }

    if(miniflac_picture_description_string(decoder,&mem->buffer[mem->pos],mem->len,&used,string_buffer,string_buffer_len+1,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    string_buffer[temp32] = '\0';
    fprintf(stdout,"%s\n",string_buffer);

    if(miniflac_picture_width(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  width=%u\n",temp32);

    if(miniflac_picture_height(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  height=%u\n",temp32);

    if(miniflac_picture_colordepth(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  colordepth=%u\n",temp32);

    if(miniflac_picture_totalcolors(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  totalcolors=%u\n",temp32);

    if(miniflac_picture_length(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  data=[%u bytes]\n",temp32);

    /* notice we skip grabbing the picture data - we can stop parsing a metadata
     * block whenever we like and call miniflac_sync to move ahead to the next
     * block boundary */
    free(string_buffer);
    fflush(stdout);
}

static void
dump_cuesheet(miniflac_t* decoder, membuffer_t* mem) {
    uint32_t used = 0;
    uint8_t  temp8 = 0;
    uint32_t temp32 = 0;
    uint64_t temp64 = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int t = 0;
    char* string_buffer = NULL;
    uint32_t string_buffer_len = 0;
    MINIFLAC_RESULT res = MINIFLAC_OK;

    fprintf(stdout,"[cuesheet]\n");

    /* the catalogue string is always a 128-byte string, the
     * miniflac_cuesheet_catalogue_length function is for convenience, so
     * we can treat it the same as variable-length strings */
    if(miniflac_cuesheet_catalogue_length(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;

    if(temp32 > string_buffer_len) {
        string_buffer = realloc(string_buffer,temp32 + 1);
        if(string_buffer == NULL) abort();
        string_buffer_len = temp32;
    }

    if(miniflac_cuesheet_catalogue_string(decoder,&mem->buffer[mem->pos],mem->len,&used,string_buffer,string_buffer_len+1,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    string_buffer[temp32] = '\0';
    fprintf(stdout,"  media catalogue number: ");
    for(t=0;t<temp32;t++) {
        if(string_buffer[t] == '\0') break;
        fprintf(stdout,"%c",string_buffer[t]);
    }
    if(t == 0) {
        fprintf(stdout,"(empty)");
    }
    fprintf(stdout,"\n");

    if(miniflac_cuesheet_leadin(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp64) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  leadin: %lu\n",temp64);

    if(miniflac_cuesheet_cdflag(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp8) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  cdflag: %u\n",temp8);

    if(miniflac_cuesheet_tracks(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp8) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  tracks: %u\n",temp8);

    i = 0;
    while( (res =miniflac_cuesheet_track_offset(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp64)) == MINIFLAC_OK) {
        mem->len -= used;
        mem->pos += used;
        fprintf(stdout,"  [track %u]\n",++i);
        fprintf(stdout,"    offset: %lu\n",temp64);

        if(miniflac_cuesheet_track_number(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp8) != MINIFLAC_OK) abort();
        mem->len -= used;
        mem->pos += used;
        fprintf(stdout,"    number: %u\n",temp8);

        if(miniflac_cuesheet_track_isrc_length(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
        mem->len -= used;
        mem->pos += used;

        if(temp32 > string_buffer_len) {
            string_buffer = realloc(string_buffer,temp32 + 1);
            if(string_buffer == NULL) abort();
            string_buffer_len = temp32;
        }
        if(miniflac_cuesheet_track_isrc_string(decoder,&mem->buffer[mem->pos],mem->len,&used,string_buffer,string_buffer_len+1,&temp32) != MINIFLAC_OK) abort();
        mem->len -= used;
        mem->pos += used;
        string_buffer[temp32] = '\0';
        fprintf(stdout,"    isrc: ");
        for(t=0;t<temp32;t++) {
            if(string_buffer[t] == '\0') break;
            fprintf(stdout,"%c",string_buffer[t]);
        }
        if(t == 0) {
            fprintf(stdout,"(empty)");
        }
        fprintf(stdout,"\n");

        if(miniflac_cuesheet_track_type(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp8) != MINIFLAC_OK) abort();
        mem->len -= used;
        mem->pos += used;
        fprintf(stdout,"    type: %u\n",temp8);

        if(miniflac_cuesheet_track_preemph(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp8) != MINIFLAC_OK) abort();
        mem->len -= used;
        mem->pos += used;
        fprintf(stdout,"    preemph: %u\n",temp8);

        if(miniflac_cuesheet_track_indexpoints(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp8) != MINIFLAC_OK) abort();
        mem->len -= used;
        mem->pos += used;
        fprintf(stdout,"    indexpoints: %u\n",temp8);

        j = 0;
        while( (res = miniflac_cuesheet_index_point_offset(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp64)) == MINIFLAC_OK) {
            mem->len -= used;
            mem->pos += used;

            fprintf(stdout,"    [index point %u]\n",++j);
            fprintf(stdout,"      offset: %lu\n",temp64);

            if(miniflac_cuesheet_index_point_number(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp8) != MINIFLAC_OK) abort();
            mem->len -= used;
            mem->pos += used;
            fprintf(stdout,"      number: %u\n",temp8);
        }
        mem->len -= used;
        mem->pos += used;
        if(res != MINIFLAC_METADATA_END) abort();
    }
    mem->len -= used;
    mem->pos += used;
    if(res != MINIFLAC_METADATA_END) abort();

    free(string_buffer);
    fflush(stdout);
}

static void
dump_seektable(miniflac_t* decoder, membuffer_t* mem) {
    uint32_t used = 0;
    uint16_t temp16 = 0;
    uint64_t temp64 = 0;
    unsigned int i = 0;
    MINIFLAC_RESULT res = MINIFLAC_OK;

    fprintf(stdout,"[seektable]\n");
    while( (res = miniflac_seektable_sample_number(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp64)) == MINIFLAC_OK) {
        mem->len -= used;
        mem->pos += used;

        fprintf(stdout,"  [seekpoint %u]\n",++i);
        fprintf(stdout,"    sample number: %lu\n",temp64);

        if(miniflac_seektable_sample_offset(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp64) != MINIFLAC_OK) abort();
        mem->len -= used;
        mem->pos += used;
        fprintf(stdout,"    sample offset: %lu\n",temp64);

        if(miniflac_seektable_samples(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp16) != MINIFLAC_OK) abort();
        mem->len -= used;
        mem->pos += used;
        fprintf(stdout,"    samples: %u\n",temp16);
    }
    mem->len -= used;
    mem->pos += used;
    fflush(stdout);
    if(res != MINIFLAC_METADATA_END) abort();
}

static void
dump_application(miniflac_t* decoder, membuffer_t* mem) {
    uint32_t used;
    uint32_t temp32;

    fprintf(stdout,"[application]\n");
    if(miniflac_application_id(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  id: 0x%08x\n",temp32);

    if(miniflac_application_length(decoder,&mem->buffer[mem->pos],mem->len,&used,&temp32) != MINIFLAC_OK) abort();
    mem->len -= used;
    mem->pos += used;
    fprintf(stdout,"  length: %u bytes\n",temp32);
    fflush(stdout);

    /* another example of skipping the decode */
}

int main(int argc, const char *argv[]) {
    MINIFLAC_RESULT res;
    int r = 1;
    unsigned int i = 0;
    FILE* output = NULL;
    uint32_t used = 0;
    uint32_t sampSize = 0;
    uint32_t len = 0;
    uint32_t frameTotal = 0;
    miniflac_t* decoder = NULL;
    int32_t** samples = NULL;
    uint8_t* outSamples = NULL;


    membuffer_t mem;

    if(argc < 3) {
        fprintf(stderr,"Usage: %s /path/to/flac /path/to/pcm\n",argv[0]);
        goto cleanup;
    }

    mem.buffer = slurp(argv[1],&mem.len);
    if(mem.buffer == NULL) goto cleanup;
    mem.pos = 0;

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

    if(miniflac_sync(decoder,&mem.buffer[mem.pos],mem.len,&used) != MINIFLAC_OK) abort();
    mem.len -= used;
    mem.pos += used;

    /* work our way through the metadata frames */
    while(decoder->state == MINIFLAC_METADATA) {
        printf("metadata block: type: %u, is_last: %u, length: %u\n",
          decoder->metadata.header.type_raw,
          decoder->metadata.header.is_last,
          decoder->metadata.header.length);
        fflush(stdout);
        if(decoder->metadata.header.type == MINIFLAC_METADATA_STREAMINFO) {
            dump_streaminfo(decoder,&mem);
        }
        else if(decoder->metadata.header.type == MINIFLAC_METADATA_VORBIS_COMMENT) {
            dump_vorbis_comment(decoder,&mem);
        }
        else if(decoder->metadata.header.type == MINIFLAC_METADATA_PICTURE) {
            dump_picture(decoder,&mem);
        }
        else if(decoder->metadata.header.type == MINIFLAC_METADATA_CUESHEET) {
            dump_cuesheet(decoder,&mem);
        }
        else if(decoder->metadata.header.type == MINIFLAC_METADATA_SEEKTABLE) {
            dump_seektable(decoder,&mem);
        }
        else if(decoder->metadata.header.type == MINIFLAC_METADATA_APPLICATION) {
            dump_application(decoder,&mem);
        }

        if(miniflac_sync(decoder,&mem.buffer[mem.pos],mem.len,&used) != MINIFLAC_OK) abort();
        mem.len -= used;
        mem.pos += used;
    }

    wav_header_create(output,decoder->frame.header.sample_rate,decoder->frame.header.channels,decoder->frame.header.bps);

    /* now we're at the beginning of a frame (just past the frame header) and can start decoding */
    while( (res = miniflac_decode(decoder,&mem.buffer[mem.pos],mem.len,&used,samples)) == MINIFLAC_OK) {
        mem.len -= used;
        mem.pos += used;

        len = 0;
        sampSize = 0;
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
        if(frameTotal % 10 == 0) {
            fprintf(stderr,"decoded %u frames\n",frameTotal);
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
    if(mem.buffer != NULL) free(mem.buffer);
    if(decoder != NULL) free(decoder);

    return r;
}
