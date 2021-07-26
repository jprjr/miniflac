/* SPDX-License-Identifier: 0BSD */
#define MINIFLAC_IMPLEMENTATION
#include "../miniflac.h"
#include "slurp.h"
#include "tictoc.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FLAC/stream_decoder.h>

struct myflac_keeper {
    uint8_t* flac_data;
    uint32_t pos;
    uint32_t length;
};

static
FLAC__StreamDecoderReadStatus read_cb(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data) {
    struct myflac_keeper *k = (struct myflac_keeper *)client_data;
    size_t len = *bytes;
    (void)decoder;

    if(len > k->length) len = k->length;

    memcpy(buffer,&k->flac_data[k->pos],len);

    k->length -= len;
    k->pos += len;
    *bytes = len;

    if(k->length > 0) return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
}

static
FLAC__StreamDecoderWriteStatus write_cb(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data) {
    (void)decoder;
    (void)frame;
    (void)buffer;
    (void)client_data;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static
void error_cb(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
    (void)decoder;
    (void)status;
    (void)client_data;
}

int main(int argc, const char* argv[]) {
    MINIFLAC_RESULT res;
    miniflac_t decoder;
    FLAC__StreamDecoder* flac_decoder;
    TicTocTimer t;

    struct myflac_keeper k;

    uint32_t i = 0;
    uint8_t* flac_data = NULL;
    uint32_t flac_len = 0;
    uint32_t pos = 0;
    uint32_t used = 0;
    uint32_t length = 0;
    int32_t** samples = NULL;
    unsigned int channels = 0;
    unsigned int sample_rate = 0;
    drflac_uint64 total_pcm_frame_count = 0;

    float elapsed_miniflac = 0.0f;
    float elapsed_libflac = 0.0f;
    float elapsed_drflac = 0.0f;

    if(argc < 2) {
        fprintf(stderr,"Usage: %s /path/to/flac\n",argv[0]);
        return 1;
    }

    flac_data = slurp(argv[1],&flac_len);
    if(flac_data == NULL) return 1;

    samples = (int32_t**)malloc(sizeof(int32_t*) * 8);
    if(samples == NULL) return 1;
    for(i=0;i<8;i++) {
        samples[i] = (int32_t*)malloc(sizeof(int32_t) * 65535);
        if(samples[i] == NULL) return 1;
    }

    length = flac_len;

    t = tic();
    miniflac_init(&decoder, MINIFLAC_CONTAINER_UNKNOWN);
    while( (res = miniflac_decode(&decoder,&flac_data[pos],length,&used,samples)) == MINIFLAC_OK) {
        length -= used;
        pos += used;
    }
    elapsed_miniflac = toc(&t);

    flac_decoder = FLAC__stream_decoder_new();

    k.flac_data = flac_data;
    k.length = flac_len;
    k.pos = 0;

    if(FLAC__stream_decoder_init_stream(flac_decoder,read_cb,NULL,NULL,NULL,NULL,write_cb,NULL,error_cb,&k) != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        fprintf(stderr,"error initializing FLAC decoder\n");
        return 1;
    }

    toc(&t);
    FLAC__stream_decoder_process_until_end_of_stream(flac_decoder);
    elapsed_libflac = toc(&t);

    drflac_open_memory_and_read_pcm_frames_s32(flac_data,flac_len, &channels, &sample_rate, &total_pcm_frame_count, NULL);
    elapsed_drflac = toc(&t);

    fprintf(stderr,"libflac elapsed time: %f seconds\n",elapsed_libflac);
    fprintf(stderr,"drflac elapsed time: %f seconds\n",elapsed_drflac);
    fprintf(stderr,"miniflac elapsed time: %f seconds\n",elapsed_miniflac);
    free(flac_data);
    for(i=0;i<8;i++) {
        free(samples[i]);
    }
    free(samples);
    return 0;
}
