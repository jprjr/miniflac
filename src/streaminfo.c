/* SPDX-License-Identifier: 0BSD */
#include "streaminfo.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_streaminfo_init(miniflac_streaminfo_t* streaminfo) {
    streaminfo->state = MINIFLAC_STREAMINFO_MINBLOCKSIZE;
    streaminfo->pos = 0;
    streaminfo->sample_rate = 0;
    streaminfo->bps = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_min_block_size(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint16_t* min_block_size) {
    uint16_t t;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: {
            if(miniflac_bitreader_fill_nocrc(br,16)) return MINIFLAC_CONTINUE;
            t = (uint16_t) miniflac_bitreader_read(br,16);
            if(min_block_size != NULL) {
                *min_block_size = t;
            }
            streaminfo->state = MINIFLAC_STREAMINFO_MAXBLOCKSIZE;
            return MINIFLAC_OK;
        }
        default: break;
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_max_block_size(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint16_t* max_block_size) {
    uint16_t t;
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: {
            r = miniflac_streaminfo_read_min_block_size(streaminfo,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: {
            if(miniflac_bitreader_fill_nocrc(br,16)) return MINIFLAC_CONTINUE;
            t = (uint16_t) miniflac_bitreader_read(br,16);
            if(max_block_size != NULL) {
                *max_block_size = t;
            }
            streaminfo->state = MINIFLAC_STREAMINFO_MINFRAMESIZE;
            return MINIFLAC_OK;
        }
        default: break;
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_min_frame_size(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint32_t* min_frame_size) {
    uint32_t t;
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: {
            r = miniflac_streaminfo_read_max_block_size(streaminfo,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: {
            if(miniflac_bitreader_fill_nocrc(br,24)) return MINIFLAC_CONTINUE;
            t = (uint32_t) miniflac_bitreader_read(br,24);
            if(min_frame_size != NULL) {
                *min_frame_size = t;
            }
            streaminfo->state = MINIFLAC_STREAMINFO_MAXFRAMESIZE;
            return MINIFLAC_OK;
        }
        default: break;
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_max_frame_size(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint32_t* max_frame_size) {
    uint32_t t;
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: {
            r = miniflac_streaminfo_read_min_frame_size(streaminfo,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MAXFRAMESIZE: {
            if(miniflac_bitreader_fill_nocrc(br,24)) return MINIFLAC_CONTINUE;
            t = (uint32_t) miniflac_bitreader_read(br,24);
            if(max_frame_size != NULL) {
                *max_frame_size = t;
            }
            streaminfo->state = MINIFLAC_STREAMINFO_SAMPLERATE;
            return MINIFLAC_OK;
        }
        default: break;
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_sample_rate(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint32_t* sample_rate) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXFRAMESIZE: {
            r = miniflac_streaminfo_read_max_frame_size(streaminfo,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_SAMPLERATE: {
            if(miniflac_bitreader_fill_nocrc(br,20)) return MINIFLAC_CONTINUE;
            streaminfo->sample_rate = (uint32_t) miniflac_bitreader_read(br,20);
            if(sample_rate != NULL) {
                *sample_rate = streaminfo->sample_rate;
            }
            streaminfo->state = MINIFLAC_STREAMINFO_CHANNELS;
            return MINIFLAC_OK;
        }
        default: break;
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_channels(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint8_t* channels) {
    uint8_t t;
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_SAMPLERATE: {
            r = miniflac_streaminfo_read_sample_rate(streaminfo,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_CHANNELS: {
            if(miniflac_bitreader_fill_nocrc(br,3)) return MINIFLAC_CONTINUE;
            t = (uint8_t) miniflac_bitreader_read(br,3) + 1;
            if(channels != NULL) {
                *channels = t;
            }
            streaminfo->state = MINIFLAC_STREAMINFO_BPS;
            return MINIFLAC_OK;
        }
        default: break;
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_bps(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint8_t* bps) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_SAMPLERATE: /* fall-through */
        case MINIFLAC_STREAMINFO_CHANNELS: {
            r = miniflac_streaminfo_read_channels(streaminfo,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_BPS: {
            if(miniflac_bitreader_fill_nocrc(br,5)) return MINIFLAC_CONTINUE;
            streaminfo->bps = (uint8_t) miniflac_bitreader_read(br,5) + 1;
            if(bps != NULL) {
                *bps = streaminfo->bps;
            }
            streaminfo->state = MINIFLAC_STREAMINFO_TOTALSAMPLES;
            return MINIFLAC_OK;
        }
        default: break;
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_total_samples(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint64_t* total_samples) {
    uint64_t t;
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_SAMPLERATE: /* fall-through */
        case MINIFLAC_STREAMINFO_CHANNELS: /* fall-through */
        case MINIFLAC_STREAMINFO_BPS: {
            r = miniflac_streaminfo_read_bps(streaminfo,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_TOTALSAMPLES: {
            if(miniflac_bitreader_fill_nocrc(br,36)) return MINIFLAC_CONTINUE;
            t = (uint64_t) miniflac_bitreader_read(br,36);
            if(total_samples != NULL) {
                *total_samples = t;
            }
            streaminfo->state = MINIFLAC_STREAMINFO_MD5;
            return MINIFLAC_OK;
        }
        default: break;
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_md5_length(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint32_t* md5_len) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_SAMPLERATE: /* fall-through */
        case MINIFLAC_STREAMINFO_CHANNELS: /* fall-through */
        case MINIFLAC_STREAMINFO_BPS: /* fall-through */
        case MINIFLAC_STREAMINFO_TOTALSAMPLES: {
            r = miniflac_streaminfo_read_total_samples(streaminfo,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        default: break;
    }
    if(md5_len != NULL) {
        *md5_len = 16;
    }
    return MINIFLAC_OK;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_md5_data(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint8_t* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint8_t t;
    switch(streaminfo->state) {
        case MINIFLAC_STREAMINFO_MINBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXBLOCKSIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MINFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_MAXFRAMESIZE: /* fall-through */
        case MINIFLAC_STREAMINFO_SAMPLERATE: /* fall-through */
        case MINIFLAC_STREAMINFO_CHANNELS: /* fall-through */
        case MINIFLAC_STREAMINFO_BPS: /* fall-through */
        case MINIFLAC_STREAMINFO_TOTALSAMPLES: {
            r = miniflac_streaminfo_read_total_samples(streaminfo,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_STREAMINFO_MD5: {
            if(streaminfo->pos == 16) return MINIFLAC_METADATA_END;
            while(streaminfo->pos < 16) {
                if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
                t = (uint8_t)miniflac_bitreader_read(br,8);
                if(output != NULL && streaminfo->pos < length) {
                    output[streaminfo->pos] = t;
                }
                streaminfo->pos++;
            }
            if(outlen != NULL) {
                *outlen = 16 < length ? 16 : length;
            }
            return MINIFLAC_OK;
        }
        default: break;
    }

    miniflac_abort();
    return MINIFLAC_ERROR;
}
