#include "cuesheet.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_cuesheet_init(miniflac_cuesheet_t* cuesheet) {
    cuesheet->state = MINIFLAC_CUESHEET_CATALOG;
    cuesheet->pos = 0;
    cuesheet->track = 0;
    cuesheet->tracks = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_catalogue(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen) {
    char c;

    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: {
            while(cuesheet->pos < 128) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && cuesheet->pos < length) {
                    output[cuesheet->pos] = c;
                }
                cuesheet->pos++;
            }
            if(outlen != NULL) {
                *outlen = cuesheet->pos < length ? cuesheet->pos : length;
            }
            cuesheet->pos = 0;
            cuesheet->state = MINIFLAC_CUESHEET_LEADIN;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_leadin(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint64_t* leadin) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint64_t t = 0;

    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: {
            r = miniflac_cuesheet_read_catalogue(cuesheet,br, NULL, 0, NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: {
            if(miniflac_bitreader_fill(br,64)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,64);
            if(leadin != NULL) {
                *leadin = t;
            }
            cuesheet->state = MINIFLAC_CUESHEET_CDFLAG;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_cdflag(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* flag) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint8_t f = 0;

    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: {
            r = miniflac_cuesheet_read_leadin(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            f = (uint8_t)miniflac_bitreader_read(br,1);
            if(flag != NULL) {
                *flag = f;
            }
            miniflac_bitreader_discard(br,7);
            cuesheet->state = MINIFLAC_CUESHEET_SHEET_RESERVE;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_tracks(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* tracks) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: {
            r = miniflac_cuesheet_read_cdflag(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_SHEET_RESERVE: {
            while(cuesheet->pos < 258) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                miniflac_bitreader_discard(br,8);
                cuesheet->pos++;
            }
            cuesheet->pos = 0;
            cuesheet->state = MINIFLAC_CUESHEET_TRACKS;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_TRACKS: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            cuesheet->tracks = (uint8_t)miniflac_bitreader_read(br,8);
            if(tracks != NULL) {
                *tracks = cuesheet->tracks;
            }
            cuesheet->track = 0;
            cuesheet->state = MINIFLAC_CUESHEET_TRACKOFFSET;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_offset(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint64_t* track_offset) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint64_t t = 0;
    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: /* fall-through */
        case MINIFLAC_CUESHEET_SHEET_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKS: {
            r = miniflac_cuesheet_read_tracks(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_TRACKOFFSET: {
            if(cuesheet->track == cuesheet->tracks) return MINIFLAC_METADATA_END;
            if(miniflac_bitreader_fill(br,64)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,64);
            if(track_offset != NULL) {
                *track_offset = t;
            }
            cuesheet->state = MINIFLAC_CUESHEET_TRACKNUMBER;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_number(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* track_number) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint8_t t = 0;
    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: /* fall-through */
        case MINIFLAC_CUESHEET_SHEET_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKS: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKOFFSET: {
            r = miniflac_cuesheet_read_track_offset(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_TRACKNUMBER: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = (uint8_t)miniflac_bitreader_read(br,8);
            if(track_number != NULL) {
                *track_number = t;
            }
            cuesheet->pos = 0;
            cuesheet->state = MINIFLAC_CUESHEET_TRACKISRC;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_isrc(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    char c;
    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: /* fall-through */
        case MINIFLAC_CUESHEET_SHEET_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKS: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKOFFSET: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKNUMBER: {
            r = miniflac_cuesheet_read_track_number(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_TRACKISRC: {
            while(cuesheet->pos < 12) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && cuesheet->pos < length) {
                    output[cuesheet->pos] = c;
                }
                cuesheet->pos++;
            }
            if(outlen != NULL) {
                *outlen = cuesheet->pos < length ? cuesheet->pos : length;
            }
            cuesheet->pos = 0;
            cuesheet->state = MINIFLAC_CUESHEET_TRACKTYPE;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_type(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* track_type) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint8_t f = 0;

    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: /* fall-through */
        case MINIFLAC_CUESHEET_SHEET_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKS: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKOFFSET: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKNUMBER: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKISRC: {
            r = miniflac_cuesheet_read_track_isrc(cuesheet,br,NULL,0,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_TRACKTYPE: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            f = (uint8_t)miniflac_bitreader_read(br,1);
            if(track_type != NULL) {
                *track_type = f;
            }
            cuesheet->state = MINIFLAC_CUESHEET_TRACKPREEMPH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_preemph(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* track_preemph) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint8_t f = 0;

    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: /* fall-through */
        case MINIFLAC_CUESHEET_SHEET_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKS: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKOFFSET: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKNUMBER: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKISRC: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKTYPE: {
            r = miniflac_cuesheet_read_track_type(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_TRACKPREEMPH: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            f = (uint8_t)miniflac_bitreader_read(br,1);
            if(track_preemph != NULL) {
                *track_preemph = f;
            }
            miniflac_bitreader_discard(br,6);
            cuesheet->pos = 0;
            cuesheet->state = MINIFLAC_CUESHEET_TRACK_RESERVE;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_indexpoints(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* track_indexpoints) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;

    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_INDEX_OFFSET: /* fall-through */
        case MINIFLAC_CUESHEET_INDEX_NUMBER: /* fall-through */
        case MINIFLAC_CUESHEET_INDEX_RESERVE: {
            /* finish reading indexpoints */
            while(cuesheet->state != MINIFLAC_CUESHEET_TRACKOFFSET) {
                r = miniflac_cuesheet_read_index_point_offset(cuesheet,br,NULL);
                if(r != MINIFLAC_OK) break;
            }
            if(r != MINIFLAC_METADATA_END) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: /* fall-through */
        case MINIFLAC_CUESHEET_SHEET_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKS: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKOFFSET: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKNUMBER: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKISRC: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKTYPE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKPREEMPH: {
            r = miniflac_cuesheet_read_track_preemph(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_TRACK_RESERVE: {
            while(cuesheet->pos < 13) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                miniflac_bitreader_discard(br,8);
                cuesheet->pos++;
            }
            cuesheet->state = MINIFLAC_CUESHEET_TRACKPOINTS;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_TRACKPOINTS: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            cuesheet->points = (uint8_t)miniflac_bitreader_read(br,8);
            if(track_indexpoints != NULL) {
                *track_indexpoints = cuesheet->points;
            }
            cuesheet->point = 0;
            cuesheet->state = MINIFLAC_CUESHEET_INDEX_OFFSET;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_index_point_offset(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint64_t* index_point_offset) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint64_t t = 0;

    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_INDEX_NUMBER: {
            r = miniflac_cuesheet_read_index_point_number(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_INDEX_RESERVE: {
            while(cuesheet->pos < 3) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                miniflac_bitreader_discard(br,8);
                cuesheet->pos++;
            }
            cuesheet->point++;
            cuesheet->state = MINIFLAC_CUESHEET_INDEX_OFFSET;
            goto case_miniflac_cuesheet_index_offset;
        }
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: /* fall-through */
        case MINIFLAC_CUESHEET_SHEET_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKS: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKOFFSET: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKNUMBER: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKISRC: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKTYPE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKPREEMPH: /* fall-through */
        case MINIFLAC_CUESHEET_TRACK_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKPOINTS: {
            r = miniflac_cuesheet_read_track_indexpoints(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_INDEX_OFFSET: {
            case_miniflac_cuesheet_index_offset:
            if(cuesheet->point == cuesheet->points) {
                /* done reading track */
                cuesheet->track++;
                cuesheet->state = MINIFLAC_CUESHEET_TRACKOFFSET;
                return MINIFLAC_METADATA_END;
            }
            if(miniflac_bitreader_fill(br,64)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,64);
            if(index_point_offset != NULL) {
                *index_point_offset = t;
            }
            cuesheet->state = MINIFLAC_CUESHEET_INDEX_NUMBER;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_index_point_number(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* index_point_number) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint8_t t = 0;

    switch(cuesheet->state) {
        case MINIFLAC_CUESHEET_CATALOG: /* fall-through */
        case MINIFLAC_CUESHEET_LEADIN: /* fall-through */
        case MINIFLAC_CUESHEET_CDFLAG: /* fall-through */
        case MINIFLAC_CUESHEET_SHEET_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKS: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKOFFSET: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKNUMBER: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKISRC: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKTYPE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKPREEMPH: /* fall-through */
        case MINIFLAC_CUESHEET_TRACK_RESERVE: /* fall-through */
        case MINIFLAC_CUESHEET_TRACKPOINTS:
        case MINIFLAC_CUESHEET_INDEX_OFFSET: {
            r = miniflac_cuesheet_read_index_point_offset(cuesheet,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_CUESHEET_INDEX_NUMBER: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            if(index_point_number != NULL) {
                *index_point_number = t;
            }
            cuesheet->pos = 0;
            cuesheet->state = MINIFLAC_CUESHEET_INDEX_RESERVE;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}
