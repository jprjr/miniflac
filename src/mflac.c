#include "mflac.h"

#define MFLAC_PASTE(a,b) a ## b

#define MFLAC_FUNC_BODY(a) \
    while( (res = a) == MINIFLAC_CONTINUE ) { \
        received = m->read(m->buffer, MFLAC_BUFFER_SIZE, m->userdata); \
        if(received == 0) return MFLAC_EOF; \
        m->buflen = received; \
        m->bufpos = 0; \
    } \
    if(res < MINIFLAC_OK) { \
        return (MFLAC_RESULT)res; \
    } \
    m->bufpos += used; \
    m->buflen -= used;


#define MFLAC_GET0_BODY(var) MFLAC_FUNC_BODY(miniflac_ ## var (&m->flac, &m->buffer[m->bufpos], m->buflen, &used) )
#define MFLAC_GET1_BODY(var, a) MFLAC_FUNC_BODY(miniflac_ ## var(&m->flac, &m->buffer[m->bufpos], m->buflen, &used, a) )
#define MFLAC_GET3_BODY(var, a, b, c) MFLAC_FUNC_BODY(miniflac_ ## var(&m->flac, &m->buffer[m->bufpos], m->buflen, &used, a, b, c) )

#define MFLAC_FUNC(sig,body) \
MINIFLAC_API \
MFLAC_RESULT \
sig { \
    MINIFLAC_RESULT res = MINIFLAC_OK; \
    uint32_t used = 0; \
    size_t received = 0; \
    body \
    return (MFLAC_RESULT)res; \
}

#define MFLAC_GET0_FUNC(var) MFLAC_FUNC(MFLAC_PASTE(mflac_,var)(mflac_t* m),MFLAC_GET0_BODY(var))
#define MFLAC_GET1_FUNC(var, typ) MFLAC_FUNC(MFLAC_PASTE(mflac_,var)(mflac_t* m, typ p1),MFLAC_GET1_BODY(var, p1))
#define MFLAC_GET3_FUNC(var, typ) MFLAC_FUNC(MFLAC_PASTE(mflac_,var)(mflac_t* m, typ p1, uint32_t p2, uint32_t* p3),MFLAC_GET3_BODY(var, p1, p2, p3))

MINIFLAC_API
MINIFLAC_CONST
size_t
mflac_size(void) {
    return sizeof(mflac_t);
}

MINIFLAC_API
void
mflac_init(mflac_t* m, MINIFLAC_CONTAINER container, mflac_readcb read, void *userdata) {
    miniflac_init(&m->flac, container);
    m->read = read;
    m->userdata = userdata;
    m->bufpos = 0;
    m->buflen = 0;
}

MINIFLAC_API
void
mflac_reset(mflac_t* m, MINIFLAC_STATE state) {
    miniflac_reset(&m->flac, state);
    m->bufpos = 0;
    m->buflen = 0;
}

MFLAC_GET0_FUNC(sync)

MFLAC_GET1_FUNC(decode,int32_t**)

MFLAC_GET1_FUNC(streaminfo_min_block_size, uint16_t*)
MFLAC_GET1_FUNC(streaminfo_max_block_size, uint16_t*)
MFLAC_GET1_FUNC(streaminfo_min_frame_size, uint32_t*)
MFLAC_GET1_FUNC(streaminfo_max_frame_size, uint32_t*)
MFLAC_GET1_FUNC(streaminfo_sample_rate, uint32_t*)
MFLAC_GET1_FUNC(streaminfo_channels, uint8_t*)
MFLAC_GET1_FUNC(streaminfo_bps, uint8_t*)
MFLAC_GET1_FUNC(streaminfo_total_samples, uint64_t*)
MFLAC_GET1_FUNC(streaminfo_md5_length, uint32_t*)
MFLAC_GET3_FUNC(streaminfo_md5_data, uint8_t*)

MFLAC_GET1_FUNC(vorbis_comment_vendor_length, uint32_t*)
MFLAC_GET3_FUNC(vorbis_comment_vendor_string, char*)
MFLAC_GET1_FUNC(vorbis_comment_total, uint32_t*)
MFLAC_GET1_FUNC(vorbis_comment_length, uint32_t*)
MFLAC_GET3_FUNC(vorbis_comment_string, char*)

MFLAC_GET1_FUNC(padding_length, uint32_t*)
MFLAC_GET3_FUNC(padding_data, uint8_t*)

MFLAC_GET1_FUNC(application_id, uint32_t*)
MFLAC_GET1_FUNC(application_length, uint32_t*)
MFLAC_GET3_FUNC(application_data, uint8_t*)

MFLAC_GET1_FUNC(seektable_seekpoints, uint32_t*)
MFLAC_GET1_FUNC(seektable_sample_number, uint64_t*)
MFLAC_GET1_FUNC(seektable_sample_offset, uint64_t*)
MFLAC_GET1_FUNC(seektable_samples, uint16_t*)

MFLAC_GET1_FUNC(cuesheet_catalog_length, uint32_t*)
MFLAC_GET3_FUNC(cuesheet_catalog_string, char*)
MFLAC_GET1_FUNC(cuesheet_leadin, uint64_t*)
MFLAC_GET1_FUNC(cuesheet_cd_flag, uint8_t*)
MFLAC_GET1_FUNC(cuesheet_tracks, uint8_t*)
MFLAC_GET1_FUNC(cuesheet_track_offset, uint64_t*)
MFLAC_GET1_FUNC(cuesheet_track_number, uint8_t*)
MFLAC_GET1_FUNC(cuesheet_track_isrc_length, uint32_t*)
MFLAC_GET3_FUNC(cuesheet_track_isrc_string, char*)
MFLAC_GET1_FUNC(cuesheet_track_audio_flag, uint8_t*)
MFLAC_GET1_FUNC(cuesheet_track_preemph_flag, uint8_t*)
MFLAC_GET1_FUNC(cuesheet_track_indexpoints, uint8_t*)
MFLAC_GET1_FUNC(cuesheet_index_point_offset, uint64_t*)
MFLAC_GET1_FUNC(cuesheet_index_point_number, uint8_t*)

MFLAC_GET1_FUNC(picture_type, uint32_t*)
MFLAC_GET1_FUNC(picture_mime_length, uint32_t*)
MFLAC_GET3_FUNC(picture_mime_string, char*)
MFLAC_GET1_FUNC(picture_description_length, uint32_t*)
MFLAC_GET3_FUNC(picture_description_string, char*)
MFLAC_GET1_FUNC(picture_width, uint32_t*)
MFLAC_GET1_FUNC(picture_height, uint32_t*)
MFLAC_GET1_FUNC(picture_colordepth, uint32_t*)
MFLAC_GET1_FUNC(picture_totalcolors, uint32_t*)
MFLAC_GET1_FUNC(picture_length, uint32_t*)
MFLAC_GET3_FUNC(picture_data, uint8_t*)

MINIFLAC_API
uint8_t
mflac_is_frame(mflac_t* m) {
    return m->flac.state == MINIFLAC_FRAME;
}

MINIFLAC_API
uint8_t
mflac_is_metadata(mflac_t* m) {
    return m->flac.state == MINIFLAC_METADATA;
}

MINIFLAC_API
uint8_t
mflac_metadata_is_last(mflac_t* m) {
    return m->flac.metadata.header.is_last;
}

MINIFLAC_API
MINIFLAC_METADATA_TYPE
mflac_metadata_type(mflac_t* m) {
    return m->flac.metadata.header.type;
}

MINIFLAC_API
uint32_t
mflac_metadata_length(mflac_t* m) {
    return m->flac.metadata.header.length;
}

MINIFLAC_API
uint8_t
mflac_metadata_is_streaminfo(mflac_t* m) {
    return m->flac.metadata.header.type == MINIFLAC_METADATA_STREAMINFO;
}

MINIFLAC_API
uint8_t
mflac_metadata_is_padding(mflac_t* m) {
    return m->flac.metadata.header.type == MINIFLAC_METADATA_PADDING;
}

MINIFLAC_API
uint8_t
mflac_metadata_is_application(mflac_t* m) {
    return m->flac.metadata.header.type == MINIFLAC_METADATA_APPLICATION;
}

MINIFLAC_API
uint8_t
mflac_metadata_is_seektable(mflac_t* m) {
    return m->flac.metadata.header.type == MINIFLAC_METADATA_SEEKTABLE;
}

MINIFLAC_API
uint8_t
mflac_metadata_is_vorbis_comment(mflac_t* m) {
    return m->flac.metadata.header.type == MINIFLAC_METADATA_VORBIS_COMMENT;
}

MINIFLAC_API
uint8_t
mflac_metadata_is_cuesheet(mflac_t* m) {
    return m->flac.metadata.header.type == MINIFLAC_METADATA_CUESHEET;
}

MINIFLAC_API
uint8_t
mflac_metadata_is_picture(mflac_t* m) {
    return m->flac.metadata.header.type == MINIFLAC_METADATA_PICTURE;
}

MINIFLAC_API
uint8_t
mflac_frame_blocking_strategy(mflac_t* m) {
    return m->flac.frame.header.blocking_strategy;
}

MINIFLAC_API
uint16_t
mflac_frame_block_size(mflac_t* m) {
    return m->flac.frame.header.block_size;
}

MINIFLAC_API
uint32_t
mflac_frame_sample_rate(mflac_t* m) {
    return m->flac.frame.header.sample_rate;
}

MINIFLAC_API
uint8_t
mflac_frame_channels(mflac_t* m) {
    return m->flac.frame.header.channels;
}

MINIFLAC_API
uint8_t
mflac_frame_bps(mflac_t* m) {
    return m->flac.frame.header.bps;
}

MINIFLAC_API
uint64_t
mflac_frame_sample_number(mflac_t* m) {
    return m->flac.frame.header.sample_number;
}

MINIFLAC_API
uint32_t
mflac_frame_frame_number(mflac_t* m) {
    return m->flac.frame.header.frame_number;
}

MINIFLAC_API
unsigned int
mflac_version_major(void) {
    return miniflac_version_major();
}

MINIFLAC_API
unsigned int
mflac_version_minor(void) {
    return miniflac_version_minor();
}

MINIFLAC_API
unsigned int
mflac_version_patch(void) {
    return miniflac_version_patch();
}

MINIFLAC_API
const char*
mflac_version_string(void) {
    return miniflac_version_string();
}

#undef MFLAC_PASTE
#undef MFLAC_FUNC_BODY
#undef MFLAC_GET0_BODY
#undef MFLAC_GET1_BODY
#undef MFLAC_GET3_BODY
#undef MFLAC_FUNC
#undef MFLAC_GET0_FUNC
#undef MFLAC_GET1_FUNC
#undef MFLAC_GET3_FUNC
