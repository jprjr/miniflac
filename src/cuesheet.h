/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_CUESHEET_H
#define MINIFLAC_CUESHEET_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_cuesheet_s miniflac_cuesheet_t;
typedef enum MINIFLAC_CUESHEET_STATE MINIFLAC_CUESHEET_STATE;

enum MINIFLAC_CUESHEET_STATE {
    MINIFLAC_CUESHEET_CATALOG,
    MINIFLAC_CUESHEET_LEADIN,
    MINIFLAC_CUESHEET_CDFLAG,
    MINIFLAC_CUESHEET_SHEET_RESERVE,
    MINIFLAC_CUESHEET_TRACKS,
    MINIFLAC_CUESHEET_TRACKOFFSET,
    MINIFLAC_CUESHEET_TRACKNUMBER,
    MINIFLAC_CUESHEET_TRACKISRC,
    MINIFLAC_CUESHEET_TRACKTYPE,
    MINIFLAC_CUESHEET_TRACKPREEMPH,
    MINIFLAC_CUESHEET_TRACK_RESERVE,
    MINIFLAC_CUESHEET_TRACKPOINTS,
    MINIFLAC_CUESHEET_INDEX_OFFSET,
    MINIFLAC_CUESHEET_INDEX_NUMBER,
    MINIFLAC_CUESHEET_INDEX_RESERVE,
};

struct miniflac_cuesheet_s {
    MINIFLAC_CUESHEET_STATE state;
    uint32_t pos;
    uint8_t track;
    uint8_t tracks;
    uint8_t point;
    uint8_t points;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_cuesheet_init(miniflac_cuesheet_t* cuesheet);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_catalog_length(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint32_t* catalog_length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_catalog_string(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_leadin(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint64_t* leadin);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_cd_flag(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* cd_flag);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_tracks(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* tracks);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_offset(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint64_t* track_offset);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_number(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* track_number);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_isrc_length(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint32_t* track_isrc_length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_isrc_string(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, char* output, uint32_t length, uint32_t* outlen);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_audio_flag(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* track_audio_flag);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_preemph_flag(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* track_preemph_flag);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_track_indexpoints(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* track_indexpoints);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_index_point_offset(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint64_t* index_point_offset);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_cuesheet_read_index_point_number(miniflac_cuesheet_t* cuesheet, miniflac_bitreader_t* br, uint8_t* index_point_number);

#ifdef __cplusplus
}
#endif

#endif
