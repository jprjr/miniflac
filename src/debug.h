/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_DEBUG_H
#define MINIFLAC_DEBUG_H

#include "application.h"
#include "cuesheet.h"
#include "bitreader.h"
#include "ogg.h"
#include "oggheader.h"
#include "flac.h"
#include "frame.h"
#include "frameheader.h"
#include "picture.h"
#include "seektable.h"
#include "subframe.h"
#include "subframeheader.h"
#include "subframe_constant.h"
#include "subframe_verbatim.h"
#include "subframe_fixed.h"
#include "subframe_lpc.h"
#include "residual.h"
#include "streaminfo.h"
#include "streammarker.h"
#include "metadata.h"
#include "metadataheader.h"
#include "vorbiscomment.h"

#ifdef __cplusplus
extern "C" {
#endif

void
miniflac_dump_bitreader(miniflac_bitreader_t* br,uint8_t indent);

void
miniflac_dump_application(miniflac_application_t* application, uint8_t indent);

void
miniflac_dump_cuesheet(miniflac_cuesheet_t* cuesheet, uint8_t indent);

void
miniflac_dump_picture(miniflac_picture_t* picture, uint8_t indent);

void
miniflac_dump_seektable(miniflac_seektable_t* seektable, uint8_t indent);

void
miniflac_dump_vorbis_comment(miniflac_vorbis_comment_t* vorbis_comment, uint8_t indent);

void
miniflac_dump_frame(miniflac_frame_t* frame, uint8_t indent);

void
miniflac_dump_frame_header(miniflac_frame_header_t* header, uint8_t indent);

void
miniflac_dump_subframe(miniflac_subframe_t* subframe, uint8_t indent);

void
miniflac_dump_subframe_header(miniflac_subframe_header_t* header, uint8_t indent);

void
miniflac_dump_subframe_constant(miniflac_subframe_constant_t* constant, uint8_t indent);

void
miniflac_dump_subframe_verbatim(miniflac_subframe_verbatim_t* verbatim, uint8_t indent);

void
miniflac_dump_subframe_fixed(miniflac_subframe_fixed_t* fixed, uint8_t indent);

void
miniflac_dump_subframe_lpc(miniflac_subframe_lpc_t* lpc, uint8_t indent);

void
miniflac_dump_residual(miniflac_residual_t* residual, uint8_t indent);

void
miniflac_dump_metadata(miniflac_metadata_t* metadata, uint8_t indent);

void
miniflac_dump_metadata_header(miniflac_metadata_header_t* header, uint8_t indent);

void
miniflac_dump_ogg(miniflac_ogg_t* ogg, uint8_t indent);

void
miniflac_dump_oggheader(miniflac_oggheader_t* oggheader, uint8_t indent);

void
miniflac_dump_streaminfo(miniflac_streaminfo_t* streaminfo, uint8_t indent);

void
miniflac_dump_streammarker(miniflac_streammarker_t* streammarker, uint8_t indent);

void
miniflac_dump_flac(miniflac_t* pFlac, uint8_t indent);

#ifdef __cplusplus
}
#endif

#endif
