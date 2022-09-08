/* SPDX-License-Identifier: 0BSD */
#ifndef MFLAC_H
#define MFLAC_H

#include <stdint.h>
#include <stddef.h>

#include "common.h"
#include "flac.h"

typedef size_t (*mflac_readcb)(uint8_t* buffer, size_t bytes, void* userdata);

enum MFLAC_RESULT {
    MFLAC_EOF          = 0,
    MFLAC_OK           = 1,
    MFLAC_METADATA_END = 2,
};

struct mflac_s {
    struct miniflac_s flac;
    mflac_readcb read;
    void* userdata;
    size_t bufpos;
    size_t buflen;
#ifndef MFLAC_BUFFER_SIZE
#define MFLAC_BUFFER_SIZE 16384
#endif
    uint8_t buffer[MFLAC_BUFFER_SIZE];
};

typedef struct mflac_s mflac_t;
typedef enum MFLAC_RESULT MFLAC_RESULT;

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_API
MINIFLAC_PURE
size_t
mflac_size(void);

MINIFLAC_API
void
mflac_init(mflac_t* m, MINIFLAC_CONTAINER container, mflac_readcb read, void* userdata);

MINIFLAC_API
MFLAC_RESULT
mflac_sync(mflac_t* m);

MINIFLAC_API
MFLAC_RESULT
mflac_decode(mflac_t* m, int32_t** samples);

/* functions to query the state without inspecting structs,
 * only valid to call after mflac_sync returns MFLAC_OK */

MINIFLAC_API
uint8_t
mflac_is_frame(mflac_t* m);

MINIFLAC_API
uint8_t
mflac_is_metadata(mflac_t* m);

MINIFLAC_API
uint8_t
mflac_metadata_is_last(mflac_t* m);

MINIFLAC_API
MINIFLAC_METADATA_TYPE
miniflac_metadata_type(miniflac_t* m);

MINIFLAC_API
uint8_t
mflac_metadata_is_streaminfo(mflac_t* m);

MINIFLAC_API
uint8_t
mflac_metadata_is_padding(mflac_t* m);

MINIFLAC_API
uint8_t
mflac_metadata_is_application(mflac_t* m);

MINIFLAC_API
uint8_t
mflac_metadata_is_seektable(mflac_t* m);

MINIFLAC_API
uint8_t
mflac_metadata_is_vorbis_comment(mflac_t* m);

MINIFLAC_API
uint8_t
mflac_metadata_is_cuesheet(mflac_t* m);

MINIFLAC_API
uint8_t
mflac_metadata_is_picture(mflac_t* m);

/*
 * METADATA FUNCTIONS
 * ==================
 *
 * The below metadata-related functions are grouped based on metadata blocks,
 * for conveience I've listed the miniflac enum label and value for each type */


/*
 * MINIFLAC_METADATA_STREAMINFO (0)
 * ================================
 *
 * Functions are listed in the order they should be called, but you can skip
 * ones you don't need. For example, you don't have to call
 * mflac_streaminfo_min_block_size, but - if you do, you have to call it before
 * mflac_streaminfo_max_block_size.
 */

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_min_block_size(mflac_t* m, uint16_t* min_block_size);

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_max_block_size(mflac_t* m, uint16_t* max_block_size);

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_min_frame_size(mflac_t* m, uint32_t* min_frame_size);

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_max_frame_size(mflac_t* m, uint32_t* max_frame_size);

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_sample_rate(mflac_t* m, uint32_t* sample_rate);

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_channels(mflac_t* m, uint8_t* channels);

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_bps(mflac_t* m, uint8_t* bps);

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_total_samples(mflac_t* m, uint64_t* total_samples);

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_md5_length(mflac_t* m, uint32_t* md5_length);

MINIFLAC_API
MFLAC_RESULT
mflac_streaminfo_md5_data(mflac_t* m, uint8_t* buffer, uint32_t buffer_length, uint32_t* buffer_used);

/*
 * MINIFLAC_METADATA_PADDING (1)
 * =============================
 *
 * Functions are listed in the order they should be called, but you can skip
 * ones you don't need. For example, you don't have to call
 * mflac_padding_length, but - if you do, you have to call it before
 * mflac_padding_data.
 */

/* gets the length of the PADDING block */
MINIFLAC_API
MFLAC_RESULT
mflac_padding_length(mflac_t* m, uint32_t* length);

/* gets the data of the PADDING block */
MINIFLAC_API
MFLAC_RESULT
mflac_padding_data(mflac_t* m, uint8_t*buffer, uint32_t buffer_length, uint32_t* buffer_used);


/*
 * MINIFLAC_METADATA_APPLICATION (2)
 * =================================
 *
 * Functions are listed in the order they should be called, but you can skip
 * ones you don't need. For example, you don't have to call
 * mflac_application_id, but - if you do, you have to call it before
 * mflac_application_length and mflac_application_data.
 */

/* gets the id of the APPLICATION block */
MINIFLAC_API
MFLAC_RESULT
mflac_application_id(mflac_t* m, uint32_t* id);

/* gets the length of the APPLICATION block */
MINIFLAC_API
MFLAC_RESULT
mflac_application_length(mflac_t* m, uint32_t* length);

/* gets the data of the APPLICATION block */
MINIFLAC_API
MFLAC_RESULT
mflac_application_data(mflac_t* m, uint8_t*buffer, uint32_t buffer_length, uint32_t* buffer_used);

/*
 * MINIFLAC_METADATA_SEEKTABLE (3)
 * ===============================
 *
 * Functions are listed in the order they should be called, but you can skip
 * ones you don't need. For example, you don't have to call
 * mflac_seektable_seekpoints, but - if you do, you have to call it before
 * mflac_seektable_sample_number.
 */

MINIFLAC_API
MFLAC_RESULT
mflac_seektable_seekpoints(mflac_t* m, uint32_t* seekpoints);

MINIFLAC_API
MFLAC_RESULT
mflac_seektable_sample_number(mflac_t* m, uint64_t* sample_number);

MINIFLAC_API
MFLAC_RESULT
mflac_seektable_sample_offset(mflac_t* m, uint64_t* sample_offset);

MINIFLAC_API
MFLAC_RESULT
mflac_seektable_samples(mflac_t* m, uint16_t* samples);

/*
 * MINIFLAC_METADATA_VORBIS_COMMENT (4)
 * ===================================
 *
 * Functions are listed in the order they should be called, but you can skip
 * ones you don't need. For example, you don't have to call
 * mflac_vorbis_comment_vendor_length, but - if you do, you have to call it before
 * mflac_vorbis_comment_vendor_string
 */

/* gets the length of the vendor string - excludes the null terminator */
MINIFLAC_API
MFLAC_RESULT
mflac_vorbis_comment_vendor_length(mflac_t* m, uint32_t* length);

/* gets the vendor string - will not be terminated */
MINIFLAC_API
MFLAC_RESULT
mflac_vorbis_comment_vendor_string(mflac_t* m, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

/* gets the total number of comments */
MINIFLAC_API
MFLAC_RESULT
mflac_vorbis_comment_total(mflac_t* m, uint32_t* total);

/* gets the length number of the next comment - does not include null terminator */
MINIFLAC_API
MFLAC_RESULT
mflac_vorbis_comment_length(mflac_t* m, uint32_t* length);

/* gets the next comment  - will not be null-terminated! */
MINIFLAC_API
MFLAC_RESULT
mflac_vorbis_comment_string(mflac_t* m, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

/*
 * MINIFLAC_METADATA_CUESHEET (5)
 * ==============================
 *
 * Functions are listed in the order they should be called, but you can skip
 * ones you don't need. For example, you don't have to call
 * mflac_cuesheet_catalog_length, but - if you do, you have to call it before
 * mflac_cuesheet_catalog_string.
 */

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_catalog_length(mflac_t* m, uint32_t* length);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_catalog_string(mflac_t* m, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_leadin(mflac_t* m, uint64_t* leadin);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_cd_flag(mflac_t* m, uint8_t* cd_flag);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_tracks(mflac_t* m, uint8_t* tracks);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_track_offset(mflac_t* m, uint64_t* track_offset);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_track_number(mflac_t* m, uint8_t* track_number);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_track_isrc_length(mflac_t* m, uint32_t* length);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_track_isrc_string(mflac_t* m, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_track_audio_flag(mflac_t* m, uint8_t* track_audio_flag);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_track_preemph_flag(mflac_t* m, uint8_t* track_preemph_flag);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_track_indexpoints(mflac_t* m, uint8_t* track_indexpoints);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_index_point_offset(mflac_t* m, uint64_t* index_point_offset);

MINIFLAC_API
MFLAC_RESULT
mflac_cuesheet_index_point_number(mflac_t* m, uint8_t* index_point_number);

/*
 * MINIFLAC_METADATA_PICTURE (6)
 * ==============================
 *
 * Functions are listed in the order they should be called, but you can skip
 * ones you don't need. For example, you don't have to call
 * mflac_picture_type, but - if you do, you have to call it before
 * mflac_pictue_mime_length.
 */

MINIFLAC_API
MFLAC_RESULT
mflac_picture_type(mflac_t* m, uint32_t* picture_type);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_mime_length(mflac_t* m, uint32_t* picture_mime_length);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_mime_string(mflac_t* m, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_description_length(mflac_t* m, uint32_t* picture_description_length);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_description_string(mflac_t* m, char* buffer, uint32_t buffer_length, uint32_t* buffer_used);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_width(mflac_t* m, uint32_t* picture_width);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_height(mflac_t* m, uint32_t* picture_height);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_colordepth(mflac_t* m, uint32_t* picture_colordepth);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_totalcolors(mflac_t* m, uint32_t* picture_totalcolors);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_length(mflac_t* m, uint32_t* picture_length);

MINIFLAC_API
MFLAC_RESULT
mflac_picture_data(mflac_t* m, uint8_t* buffer, uint32_t buffer_length, uint32_t* buffer_used);

MINIFLAC_API
unsigned int
mflac_version_major(void);

MINIFLAC_API
unsigned int
mflac_version_minor(void);

MINIFLAC_API
unsigned int
mflac_version_patch(void);

MINIFLAC_API
const char*
mflac_version_string(void);

#ifdef __cplusplus
}
#endif

#endif
