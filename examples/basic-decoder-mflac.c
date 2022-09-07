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
 * a .wav file, using mflac rather than miniflac */

static size_t
readcb(uint8_t* buffer, size_t size, void* userdata) {
    return fread(buffer,1,size,(FILE *)userdata);
}

static void
dump_streaminfo(mflac_t* m) {
    uint8_t  temp8;
    uint16_t temp16;
    uint32_t temp32;
    uint64_t temp64;
    uint8_t md5[16];

    fprintf(stdout,"[streaminfo]\n");

    if(mflac_streaminfo_min_block_size(m,&temp16) != MFLAC_OK) abort();
    fprintf(stdout,"  min_block_size: %u\n",temp16);

    if(mflac_streaminfo_max_block_size(m,&temp16) != MFLAC_OK) abort();
    fprintf(stdout,"  max_block_size: %u\n",temp16);

    if(mflac_streaminfo_min_frame_size(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  min_frame_size: %u\n",temp16);

    if(mflac_streaminfo_max_frame_size(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  max_frame_size: %u\n",temp16);

    if(mflac_streaminfo_sample_rate(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  sample_rate: %u\n",temp32);

    if(mflac_streaminfo_channels(m,&temp8) != MFLAC_OK) abort();
    fprintf(stdout,"  channels: %u\n",temp8);

    if(mflac_streaminfo_bps(m,&temp8) != MFLAC_OK) abort();
    fprintf(stdout,"  bps: %u\n",temp8);

    if(mflac_streaminfo_total_samples(m,&temp64) != MFLAC_OK) abort();
    fprintf(stdout,"  total_samples: %lu\n",temp64);

    if(mflac_streaminfo_md5_data(m,md5,16,NULL) != MFLAC_OK) abort();
    fprintf(stdout,"  md5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
      md5[ 0], md5[ 1], md5[ 2], md5[ 3],
      md5[ 4], md5[ 5], md5[ 6], md5[ 7],
      md5[ 8], md5[ 9], md5[10], md5[11],
      md5[12], md5[13], md5[14], md5[15]);
    fflush(stdout);

}

static void
dump_vorbis_comment(mflac_t* m) {
    uint32_t temp32 = 0;
    unsigned int i = 0;
    char* string_buffer = NULL;
    uint32_t string_buffer_len = 0;
    MFLAC_RESULT res = MFLAC_OK;

    fprintf(stdout,"[vorbis_comment]\n");

    if(mflac_vorbis_comment_vendor_length(m,&temp32) != MFLAC_OK) abort();

    if(temp32 > string_buffer_len) {
        string_buffer = (char *)realloc(string_buffer,temp32+1);
        if(string_buffer == NULL) abort();
        string_buffer_len = temp32;
    }

    fprintf(stdout,"  vendor string=[%u]",temp32);
    if(mflac_vorbis_comment_vendor_string(m,string_buffer,string_buffer_len+1,&temp32) != MFLAC_OK)
        abort();
    string_buffer[temp32] = '\0';
    fprintf(stdout,"%s\n",string_buffer);

    while( (res = mflac_vorbis_comment_length(m,&temp32)) == MFLAC_OK) {
        fprintf(stdout,"  comment[%u]=[%u]",i++,temp32);
        if(temp32 > string_buffer_len) {
            string_buffer = (char *)realloc(string_buffer,temp32+1);
            if(string_buffer == NULL) abort();
            string_buffer_len = temp32;
        }
        if(mflac_vorbis_comment_string(m,string_buffer,string_buffer_len+1,&temp32) != MFLAC_OK)
            abort();
        string_buffer[temp32] = '\0';
        fprintf(stdout,"%s\n",string_buffer);
    }

    if(res != MFLAC_METADATA_END) abort();
    free(string_buffer);
    fflush(stdout);
}

static void
dump_picture(mflac_t* m) {
    uint32_t temp32 = 0;
    char* string_buffer = NULL;
    uint32_t string_buffer_len = 0;

    fprintf(stdout,"[picture]\n");
    if(mflac_picture_type(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  type=%u\n",temp32);

    if(mflac_picture_mime_length(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  mime string=[%u]",temp32);
    if(temp32 > string_buffer_len) {
        string_buffer = (char *)realloc(string_buffer,temp32 + 1);
        if(string_buffer == NULL) abort();
        string_buffer_len = temp32;
    }
    if(mflac_picture_mime_string(m,string_buffer,string_buffer_len+1,&temp32) != MFLAC_OK) abort();
    string_buffer[temp32] = '\0';
    fprintf(stdout,"%s\n",string_buffer);

    if(mflac_picture_description_length(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  description string=[%u]",temp32);
    if(temp32 > string_buffer_len) {
        string_buffer = (char *)realloc(string_buffer,temp32 + 1);
        if(string_buffer == NULL) abort();
        string_buffer_len = temp32;
    }

    if(mflac_picture_description_string(m,string_buffer,string_buffer_len+1,&temp32) != MFLAC_OK) abort();
    string_buffer[temp32] = '\0';
    fprintf(stdout,"%s\n",string_buffer);

    if(mflac_picture_width(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  width=%u\n",temp32);

    if(mflac_picture_height(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  height=%u\n",temp32);

    if(mflac_picture_colordepth(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  colordepth=%u\n",temp32);

    if(mflac_picture_totalcolors(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  totalcolors=%u\n",temp32);

    if(mflac_picture_length(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  data=[%u bytes]\n",temp32);

    /* notice we skip grabbing the picture data - we can stop parsing a metadata
     * block whenever we like and call miniflac_sync to move ahead to the next
     * block boundary */
    free(string_buffer);
    fflush(stdout);
}

static void
dump_cuesheet(mflac_t* m) {
    uint8_t  temp8 = 0;
    uint32_t temp32 = 0;
    uint64_t temp64 = 0;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int t = 0;
    char* string_buffer = NULL;
    uint32_t string_buffer_len = 0;
    MFLAC_RESULT res = MFLAC_OK;

    fprintf(stdout,"[cuesheet]\n");

    /* the catalog string is always a 128-byte string, the
     * mflac_cuesheet_catalogue_length function is for convenience, so
     * we can treat it the same as variable-length strings */
    if(mflac_cuesheet_catalog_length(m,&temp32) != MFLAC_OK) abort();

    if(temp32 > string_buffer_len) {
        string_buffer = (char *)realloc(string_buffer,temp32 + 1);
        if(string_buffer == NULL) abort();
        string_buffer_len = temp32;
    }

    if(mflac_cuesheet_catalog_string(m,string_buffer,string_buffer_len+1,&temp32) != MFLAC_OK) abort();
    string_buffer[temp32] = '\0';
    fprintf(stdout,"  media catalog number: ");
    for(t=0;t<temp32;t++) {
        if(string_buffer[t] == '\0') break;
        fprintf(stdout,"%c",string_buffer[t]);
    }
    if(t == 0) {
        fprintf(stdout,"(empty)");
    }
    fprintf(stdout,"\n");

    if(mflac_cuesheet_leadin(m,&temp64) != MFLAC_OK) abort();
    fprintf(stdout,"  leadin: %lu\n",temp64);

    if(mflac_cuesheet_cd_flag(m,&temp8) != MFLAC_OK) abort();
    fprintf(stdout,"  cd_flag: %u\n",temp8);

    if(mflac_cuesheet_tracks(m,&temp8) != MFLAC_OK) abort();
    fprintf(stdout,"  tracks: %u\n",temp8);

    i = 0;
    while( (res =mflac_cuesheet_track_offset(m,&temp64)) == MFLAC_OK) {
        fprintf(stdout,"  [track %u]\n",++i);
        fprintf(stdout,"    offset: %lu\n",temp64);

        if(mflac_cuesheet_track_number(m,&temp8) != MFLAC_OK) abort();
        fprintf(stdout,"    number: %u\n",temp8);

        if(mflac_cuesheet_track_isrc_length(m,&temp32) != MFLAC_OK) abort();

        if(temp32 > string_buffer_len) {
            string_buffer = (char *)realloc(string_buffer,temp32 + 1);
            if(string_buffer == NULL) abort();
            string_buffer_len = temp32;
        }
        if(mflac_cuesheet_track_isrc_string(m,string_buffer,string_buffer_len+1,&temp32) != MFLAC_OK) abort();
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

        if(mflac_cuesheet_track_audio_flag(m,&temp8) != MFLAC_OK) abort();
        fprintf(stdout,"    type: %u\n",temp8);

        if(mflac_cuesheet_track_preemph_flag(m,&temp8) != MFLAC_OK) abort();
        fprintf(stdout,"    preemph: %u\n",temp8);

        if(mflac_cuesheet_track_indexpoints(m,&temp8) != MFLAC_OK) abort();
        fprintf(stdout,"    indexpoints: %u\n",temp8);

        j = 0;
        while( (res = mflac_cuesheet_index_point_offset(m,&temp64)) == MFLAC_OK) {

            fprintf(stdout,"    [index point %u]\n",++j);
            fprintf(stdout,"      offset: %lu\n",temp64);

            if(mflac_cuesheet_index_point_number(m,&temp8) != MFLAC_OK) abort();
            fprintf(stdout,"      number: %u\n",temp8);
        }
        if(res != MFLAC_METADATA_END) abort();
    }
    if(res != MFLAC_METADATA_END) abort();

    free(string_buffer);
    fflush(stdout);
}

static void
dump_seektable(mflac_t* m) {
    uint16_t temp16 = 0;
    uint64_t temp64 = 0;
    unsigned int i = 0;
    MFLAC_RESULT res = MFLAC_OK;

    fprintf(stdout,"[seektable]\n");
    while( (res = mflac_seektable_sample_number(m,&temp64)) == MFLAC_OK) {
        fprintf(stdout,"  [seekpoint %u]\n",++i);
        fprintf(stdout,"    sample number: %lu\n",temp64);

        if(mflac_seektable_sample_offset(m,&temp64) != MFLAC_OK) abort();
        fprintf(stdout,"    sample offset: %lu\n",temp64);

        if(mflac_seektable_samples(m,&temp16) != MFLAC_OK) abort();
        fprintf(stdout,"    samples: %u\n",temp16);
    }
    fflush(stdout);
    if(res != MFLAC_METADATA_END) abort();
}

static void
dump_application(mflac_t* m) {
    uint32_t temp32;

    fprintf(stdout,"[application]\n");
    if(mflac_application_id(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  id: 0x%08x\n",temp32);

    if(mflac_application_length(m,&temp32) != MFLAC_OK) abort();
    fprintf(stdout,"  length: %u bytes\n",temp32);
    fflush(stdout);

    /* another example of skipping the decode */
}

int main(int argc, const char *argv[]) {
    MFLAC_RESULT res;
    mflac_t* m = NULL;

    int r = 1;
    unsigned int i = 0;
    FILE* input = NULL;
    FILE* output = NULL;
    uint32_t sampSize = 0;
    uint8_t shift = 0;
    uint32_t len = 0;
    uint32_t frameTotal = 0;
    int32_t** samples = NULL;
    uint8_t* outSamples = NULL;

    if(argc < 3) {
        fprintf(stderr,"Usage: %s /path/to/flac /path/to/wav\n",argv[0]);
        goto cleanup;
    }

    fprintf(stderr,"Built with miniflac version %s\n",miniflac_version_string());

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

    m = (mflac_t*)malloc(mflac_size());
    if(m == NULL) {
        fprintf(stderr,"Failed to allocate m\n");
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
        fprintf(stderr,"Failed to allocate pcm buffer\n");
        goto cleanup;
    }

    mflac_init(m,MINIFLAC_CONTAINER_UNKNOWN,readcb,input);

    if(mflac_sync(m) != MFLAC_OK) abort();

    /* work our way through the metadata frames */
    while(m->decoder.state == MINIFLAC_METADATA) {
        printf("metadata block: type: %u, is_last: %u, length: %u\n",
          m->decoder.metadata.header.type_raw,
          m->decoder.metadata.header.is_last,
          m->decoder.metadata.header.length);
        fflush(stdout);
        if(m->decoder.metadata.header.type == MINIFLAC_METADATA_STREAMINFO) {
            dump_streaminfo(m);
        }
        else if(m->decoder.metadata.header.type == MINIFLAC_METADATA_VORBIS_COMMENT) {
            dump_vorbis_comment(m);
        }
        else if(m->decoder.metadata.header.type == MINIFLAC_METADATA_PICTURE) {
            dump_picture(m);
        }
        else if(m->decoder.metadata.header.type == MINIFLAC_METADATA_CUESHEET) {
            dump_cuesheet(m);
        }
        else if(m->decoder.metadata.header.type == MINIFLAC_METADATA_SEEKTABLE) {
            dump_seektable(m);
        }
        else if(m->decoder.metadata.header.type == MINIFLAC_METADATA_APPLICATION) {
            dump_application(m);
        }

        if(mflac_sync(m) != MFLAC_OK) abort();
    }

    wav_header_create(output,m->decoder.frame.header.sample_rate,m->decoder.frame.header.channels,m->decoder.frame.header.bps);

    /* now we're at the beginning of a frame (just past the frame header) and can start decoding */
    while( (res = mflac_decode(m,samples)) == MFLAC_OK) {

        len = 0;
        sampSize = 0;
        packer pack = NULL;
        shift = 0;

        if(m->decoder.frame.header.bps <= 8) {
            sampSize = 1; pack = uint8_packer; shift = 8 - m->decoder.frame.header.bps;
        } else if(m->decoder.frame.header.bps <= 16) {
            sampSize = 2; pack = int16_packer; shift = 16 - m->decoder.frame.header.bps;
        } else if(m->decoder.frame.header.bps <= 24) {
            sampSize = 3; pack = int24_packer; shift = 24 - m->decoder.frame.header.bps;
        } else if(m->decoder.frame.header.bps <= 32) {
            sampSize = 4; pack = int32_packer; shift = 32 - m->decoder.frame.header.bps;
        } else  {
            abort();
        }

        len = sampSize * m->decoder.frame.header.channels * m->decoder.frame.header.block_size;

        /* samples is planar, convert into an interleaved format, and pack into little-endian */
        pack(outSamples,samples,m->decoder.frame.header.channels,m->decoder.frame.header.block_size,shift);
        fwrite(outSamples,1,len,output);
        frameTotal++;
        if(frameTotal % 10 == 0) {
            fprintf(stderr,"decoded %u frames\n",frameTotal);
        }

        /* sync up to the next frame boundary */
        res = mflac_sync(m);
        if(res != MFLAC_OK) break;
    }

    fprintf(stderr,"decoded %u frames\n",frameTotal);
    fprintf(stderr,"result: %d\n",res);
    if(res != MFLAC_EOF) {
        miniflac_dump_flac(&m->decoder,0);
    }

    assert(res == MFLAC_EOF);
    wav_header_finish(output,m->decoder.frame.header.bps);
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
    if(m != NULL) free(m);

    return r;
}
