/* SPDX-License-Identifier: 0BSD */
#include "frame.h"
#include <stddef.h>
#include <assert.h>

MINIFLAC_PRIVATE
void
miniflac_frame_init(miniflac_frame_t* frame) {
    frame->crc16 = 0;
    frame->cur_subframe = 0;
    frame->state = MINIFLAC_FRAME_HEADER;
    miniflac_frame_header_init(&frame->header);
    miniflac_subframe_init(&frame->subframe);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_frame_sync(miniflac_frame_t* frame, miniflac_bitreader_t* br, miniflac_streaminfo_t* info) {
    MINIFLAC_RESULT r;
    assert(frame->state == MINIFLAC_FRAME_HEADER);
    r = miniflac_frame_header_decode(&frame->header,br);
    if(r != MINIFLAC_OK) return r;

    if(frame->header.sample_rate == 0) {
        if(info->sample_rate == 0) return MINIFLAC_FRAME_INVALID_SAMPLE_RATE;
        frame->header.sample_rate = info->sample_rate;
    }

    if(frame->header.bps == 0) {
        if(info->bps == 0) return MINIFLAC_FRAME_INVALID_SAMPLE_SIZE;
        frame->header.bps = info->bps;
    }

    frame->state = MINIFLAC_FRAME_SUBFRAME;
    frame->cur_subframe = 0;
    miniflac_subframe_init(&frame->subframe);
    return MINIFLAC_OK;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_frame_decode(miniflac_frame_t* frame, miniflac_bitreader_t* br, miniflac_streaminfo_t* info, int32_t** output) {
    MINIFLAC_RESULT r;
    uint32_t bps;
    uint32_t i;
    uint64_t m,s;
    uint16_t t;
    switch(frame->state) {
        case MINIFLAC_FRAME_HEADER: {
            r = miniflac_frame_sync(frame,br,info);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_FRAME_SUBFRAME: {
            while(frame->cur_subframe < frame->header.channels) {
                bps = frame->header.bps;
                if(frame->header.channel_assignment == MINIFLAC_CHASSGN_LEFT_SIDE || frame->header.channel_assignment == MINIFLAC_CHASSGN_MID_SIDE) {
                    if(frame->cur_subframe == 1) bps += 1;
                } else if(frame->header.channel_assignment == MINIFLAC_CHASSGN_RIGHT_SIDE) {
                    if(frame->cur_subframe == 0) bps += 1;
                }
                r = miniflac_subframe_decode(&frame->subframe,br,output == NULL ? NULL : output[frame->cur_subframe],frame->header.block_size,bps);
                if(r != MINIFLAC_OK) return r;

                miniflac_subframe_init(&frame->subframe);
                frame->cur_subframe++;
            }

            miniflac_bitreader_align(br);
            frame->crc16 = br->crc16;
            frame->state = MINIFLAC_FRAME_FOOTER;
        }
        /* fall-through */
        case MINIFLAC_FRAME_FOOTER: {
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,16);
            if(frame->crc16 != t) {
                miniflac_abort();
                return MINIFLAC_FRAME_CRC16_INVALID;
            }
            frame->size = br->tot;
            if(output != NULL) {
                switch(frame->header.channel_assignment) {
                    case MINIFLAC_CHASSGN_LEFT_SIDE: {
                        for(i=0;i<frame->header.block_size;i++) {
                            output[1][i] = output[0][i] - output[1][i];
                        }
                        break;
                    }
                    case MINIFLAC_CHASSGN_RIGHT_SIDE: {
                        for(i=0;i<frame->header.block_size;i++) {
                            output[0][i] = output[0][i] + output[1][i];
                        }
                        break;
                    }
                    case MINIFLAC_CHASSGN_MID_SIDE: {
                        for(i=0;i<frame->header.block_size;i++) {
                            m = (uint64_t)output[0][i];
                            s = (uint64_t)output[1][i];
                            m = (m << 1) | (s & 0x01);
                            output[0][i] = (int32_t)((m + s) >> 1 );
                            output[1][i] = (int32_t)((m - s) >> 1 );
                        }
                        break;
                    }
                    default: break;
                }
            }
            break;
        }
        default: {
            /* invalid state */
            miniflac_abort();
            return MINIFLAC_ERROR;
        }
    }

    assert(br->bits == 0);
    br->crc8 = 0;
    br->crc16 = 0;
    frame->cur_subframe = 0;
    frame->state = MINIFLAC_FRAME_HEADER;
    miniflac_subframe_init(&frame->subframe);
    return MINIFLAC_OK;
}


