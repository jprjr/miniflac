/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_H
#define MINIFLAC_H

#include <stdint.h>
#include <stddef.h>

#include "common.h"
#include "ogg.h"
#include "oggheader.h"
#include "bitreader.h"
#include "streammarker.h"
#include "metadata.h"
#include "frame.h"

typedef struct miniflac_s miniflac_t;
typedef enum MINIFLAC_STATE MINIFLAC_STATE;
typedef enum MINIFLAC_CONTAINER MINIFLAC_CONTAINER;

enum MINIFLAC_STATE {
    MINIFLAC_OGGHEADER, /* will try to find an ogg header */
    MINIFLAC_STREAMMARKER_OR_FRAME, /* poke for a stream marker or audio frame 
    */
    MINIFLAC_STREAMMARKER, /* reading a stream marker */
    MINIFLAC_METADATA_OR_FRAME, /* will try to find a frame sync code or try to parse a metadata block */
    MINIFLAC_METADATA, /* currently reading a metadata block */
    MINIFLAC_FRAME,    /* currently reading an audio frame */
};

enum MINIFLAC_CONTAINER {
    MINIFLAC_CONTAINER_UNKNOWN,
    MINIFLAC_CONTAINER_NATIVE,
    MINIFLAC_CONTAINER_OGG,
};

struct miniflac_s {
    MINIFLAC_STATE state;
    MINIFLAC_CONTAINER container;
    miniflac_bitreader_t br;
    miniflac_ogg_t ogg;
    miniflac_oggheader_t oggheader;
    miniflac_streammarker_t streammarker;
    miniflac_metadata_t metadata;
    miniflac_frame_t frame;
};


#ifdef __cplusplus
extern "C" {
#endif

/* returns the number of bytes needed for the miniflac struct (for malloc, etc) */
MINIFLAC_API
MINIFLAC_PURE
size_t
miniflac_size(void);

/* give the container type if you know the kind of container,
 * otherwise 0 for unknown */
MINIFLAC_API
void
miniflac_init(miniflac_t* pFlac, MINIFLAC_CONTAINER container);

/* sync to the next metadata block or frame, parses the metadata header or frame header */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_sync(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length);

/* decode a frame of audio, automatically skips metadata if needed */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_decode(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, int32_t** samples);

/* parse a streaminfo block */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_streaminfo(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, miniflac_streaminfo_t* streaminfo);

/* get the length of the vendor string, automatically skips metadata blocks, throws an error on audio frames */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_vorbis_comment_vendor_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* vendor_length);

/* get the vendor string, automatically skips metadata blocks, throws an error on audio frames */
/* will NOT be NULL-terminated! */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_vorbis_comment_vendor_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

/* get the total number of comments, automatically skips metadata blocks, throws an error on audio frames */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_vorbis_comment_total(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* total_comments);

/* get the next comment length, automatically skips metadata blocks, throws an error on audio frames */
/* returns MINIFLAC_METADATA_END when out of comments */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_vorbis_comment_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* comment_length);

/* get the next comment string, automatically skips metadata blocks, throws an error on audio frames */
/* will NOT be NULL-terminated! */
/* returns MINIFLAC_METADATA_END when out of comments */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_vorbis_comment_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

/* read a picture type */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_type(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_type);

/* read a picture mime string length */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_mime_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_mime_length);

/* read a picture mime string */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_mime_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

/* read a picture description string length */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_description_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_description_length);

/* read a picture description string */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_description_string(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

/* read a picture width */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_width(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_width);

/* read a picture height */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_height(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_height);

/* read a picture colordepth */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_colordepth(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_colordepth);

/* read a picture totalcolors */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_totalcolors(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_totalcolors);

/* read a picture data length */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_length(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* picture_length);

/* read a picture data */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_picture_data(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* buffer, uint32_t buffer_length, uint32_t* buffer_used);

/* read a cuesheet catalogue number */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_catalogue(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* outlen);

/* read a cuesheet leadin value */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_leadin(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint64_t* leadin);

/* read a cuesheet "is this a cd" flag */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_cdflag(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* cdflag);

/* read a cuesheet total tracks */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_tracks(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* tracks);

/* read the next track offset (can return MINIFLAC_METADATA_END) */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_track_offset(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint64_t* track_offset);

/* read the next track number */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_track_number(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* track_number);

/* read the next track isrc */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_track_isrc(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, char* buffer, uint32_t buffer_length, uint32_t* outlen);

/* read the next track type flag (0 = audio, 1 = non-audio) */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_track_type(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* track_type);

/* read the track pre-emphasis flag */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_track_preemph(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* track_preemph);

/* read the total number of track index points */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_track_indexpoints(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* track_indexpoints);

/* read the next index point offset */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_index_point_offset(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint64_t* index_point_offset);

/* read the next index point number */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_cuesheet_index_point_number(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* index_point_number);

/* read the next seekpoint sample number */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_seektable_sample_number(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint64_t* sample_number);

/* read the next seekpoint sample offset */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_seektable_sample_offset(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint64_t* sample_offset);

/* read the next seekpoint # of samples in seekpoint */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_seektable_samples(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint16_t* samples);

/* read an application block's ID */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_application_id(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint32_t* id);

/* read an application block's data */
MINIFLAC_API
MINIFLAC_RESULT
miniflac_application_data(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, uint8_t* buffer, uint32_t buffer_length, uint32_t* outlen);

#ifdef __cplusplus
}
#endif

#endif
