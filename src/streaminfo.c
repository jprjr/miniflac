/* SPDX-License-Identifier: 0BSD */
#include "streaminfo.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_streaminfo_init(miniflac_streaminfo_private_t* streaminfo) {
    unsigned int i;
    streaminfo->state = MINIFLAC_STREAMINFO_MINBLOCKSIZE;
    streaminfo->info.min_block_size = 0;
    streaminfo->info.max_block_size = 0;
    streaminfo->info.min_frame_size = 0;
    streaminfo->info.max_frame_size = 0;
    streaminfo->info.sample_rate = 0;
    streaminfo->info.channels = 0;
    streaminfo->info.bps = 0;
    streaminfo->info.total_samples = 0;
    for(i=0;i<16;i++) {
        streaminfo->info.md5[i] = 0;
    }
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_streaminfo(miniflac_streaminfo_private_t* streaminfo, miniflac_bitreader_t* br, miniflac_streaminfo_t* out) {
    unsigned int i;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: {
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            streaminfo->info.min_block_size = (uint16_t) miniflac_bitreader_read(br,16);
            streaminfo->state = MINIFLAC_STREAMINFO_MAXBLOCKSIZE;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: {
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            streaminfo->info.max_block_size = (uint16_t) miniflac_bitreader_read(br,16);
            streaminfo->state = MINIFLAC_STREAMINFO_MINFRAMESIZE;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: {
            if(miniflac_bitreader_fill(br,24)) return MINIFLAC_CONTINUE;
            streaminfo->info.min_frame_size = (uint32_t) miniflac_bitreader_read(br,24);
            streaminfo->state = MINIFLAC_STREAMINFO_MAXFRAMESIZE;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MAXFRAMESIZE: {
            if(miniflac_bitreader_fill(br,24)) return MINIFLAC_CONTINUE;
            streaminfo->info.max_frame_size = (uint32_t) miniflac_bitreader_read(br,24);
            streaminfo->state = MINIFLAC_STREAMINFO_SAMPLERATE;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_SAMPLERATE: {
            if(miniflac_bitreader_fill(br,20)) return MINIFLAC_CONTINUE;
            streaminfo->info.sample_rate = (uint32_t) miniflac_bitreader_read(br,20);
            streaminfo->state = MINIFLAC_STREAMINFO_CHANNELS;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_CHANNELS: {
            if(miniflac_bitreader_fill(br,3)) return MINIFLAC_CONTINUE;
            streaminfo->info.channels = (uint8_t) miniflac_bitreader_read(br,3) + 1;
            streaminfo->state = MINIFLAC_STREAMINFO_BPS;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_BPS: {
            if(miniflac_bitreader_fill(br,5)) return MINIFLAC_CONTINUE;
            streaminfo->info.bps = (uint8_t) miniflac_bitreader_read(br,5) + 1;
            streaminfo->state = MINIFLAC_STREAMINFO_TOTALSAMPLES;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_TOTALSAMPLES: {
            if(miniflac_bitreader_fill(br,36)) return MINIFLAC_CONTINUE;
            streaminfo->info.total_samples = (uint64_t) miniflac_bitreader_read(br,36);
            streaminfo->state = MINIFLAC_STREAMINFO_MD5_1;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MD5_1: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            streaminfo->info.md5[0] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[1] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[2] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[3] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->state = MINIFLAC_STREAMINFO_MD5_2;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MD5_2: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            streaminfo->info.md5[4] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[5] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[6] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[7] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->state = MINIFLAC_STREAMINFO_MD5_3;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MD5_3: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            streaminfo->info.md5[8] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[9] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[10] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[11] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->state = MINIFLAC_STREAMINFO_MD5_4;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MD5_4: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            streaminfo->info.md5[12] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[13] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[14] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->info.md5[15] = (uint8_t) miniflac_bitreader_read(br,8);
            streaminfo->state = MINIFLAC_STREAMINFO_COMPLETE;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_COMPLETE: {
            if(out != NULL) {
                out->min_block_size = streaminfo->info.min_block_size;
                out->max_block_size = streaminfo->info.max_block_size;
                out->min_frame_size = streaminfo->info.min_frame_size;
                out->max_frame_size = streaminfo->info.max_frame_size;
                out->sample_rate = streaminfo->info.sample_rate;
                out->channels = streaminfo->info.channels;
                out->bps = streaminfo->info.bps;
                out->total_samples = streaminfo->info.total_samples;
                for(i=0;i<16;i++) {
                    out->md5[i] = streaminfo->info.md5[i];
                }
            }
            break;
        }
        default: {
            miniflac_abort();
            return MINIFLAC_ERROR;
        }
    }

    return MINIFLAC_OK;
}

