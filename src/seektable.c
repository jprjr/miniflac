#include "seektable.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_seektable_init(miniflac_seektable_t* seektable) {
    seektable->state = MINIFLAC_SEEKTABLE_SAMPLE_NUMBER;
    seektable->len = 0;
    seektable->pos = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_seektable_read_seekpoints(miniflac_seektable_t* seektable, miniflac_bitreader_t* br, uint32_t* seekpoints) {
    (void)br;

    switch(seektable->state) {
        case MINIFLAC_SEEKTABLE_SAMPLE_NUMBER: {
            if(seekpoints != NULL) {
                *seekpoints = seektable->len;
            }
            return MINIFLAC_OK;
        }
        default: break;
    }
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_seektable_read_sample_number(miniflac_seektable_t* seektable, miniflac_bitreader_t* br, uint64_t* sample_number) {
    uint64_t t = 0;

    switch(seektable->state) {
        case MINIFLAC_SEEKTABLE_SAMPLE_NUMBER: {
            if(seektable->pos == seektable->len) return MINIFLAC_METADATA_END;
            if(miniflac_bitreader_fill(br,64)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,64);
            if(sample_number != NULL) {
                *sample_number = t;
            }
            seektable->state = MINIFLAC_SEEKTABLE_SAMPLE_OFFSET;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_seektable_read_sample_offset(miniflac_seektable_t* seektable, miniflac_bitreader_t* br, uint64_t* sample_offset) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint64_t t = 0;

    switch(seektable->state) {
        case MINIFLAC_SEEKTABLE_SAMPLE_NUMBER: {
            r = miniflac_seektable_read_sample_number(seektable,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_SEEKTABLE_SAMPLE_OFFSET: {
            if(miniflac_bitreader_fill(br,64)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,64);
            if(sample_offset != NULL) {
                *sample_offset = t;
            }
            seektable->state = MINIFLAC_SEEKTABLE_SAMPLES;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_seektable_read_samples(miniflac_seektable_t* seektable, miniflac_bitreader_t* br, uint16_t* samples) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint16_t t = 0;

    switch(seektable->state) {
        case MINIFLAC_SEEKTABLE_SAMPLE_NUMBER: /* fall-through */
        case MINIFLAC_SEEKTABLE_SAMPLE_OFFSET: {
            r = miniflac_seektable_read_sample_offset(seektable,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_SEEKTABLE_SAMPLES: {
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            t = (uint16_t)miniflac_bitreader_read(br,16);
            if(samples != NULL) {
                *samples = t;
            }
            seektable->pos++;
            seektable->state = MINIFLAC_SEEKTABLE_SAMPLE_NUMBER;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}
