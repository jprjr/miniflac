/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_DEBUG_H
#define MINIFLAC_DEBUG_H

#include "bitreader.h"
#include "ogg.h"
#include "oggheader.h"
#include "flac.h"
#include "frame.h"
#include "frameheader.h"
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

#ifdef __cplusplus
extern "C" {
#endif

void
miniflac_dump_bitreader(miniflac_bitreader_t* br,uint8_t indent);

void
miniflac_dump_frame(miniflac_frame_t* frame, uint8_t indent);

void
miniflac_dump_frame_header(miniflac_frame_header* header, uint8_t indent);

void
miniflac_dump_subframe(miniflac_subframe* subframe, uint8_t indent);

void
miniflac_dump_subframe_header(miniflac_subframe_header* header, uint8_t indent);

void
miniflac_dump_subframe_constant(miniflac_subframe_constant* constant, uint8_t indent);

void
miniflac_dump_subframe_verbatim(miniflac_subframe_verbatim* verbatim, uint8_t indent);

void
miniflac_dump_subframe_fixed(miniflac_subframe_fixed* fixed, uint8_t indent);

void
miniflac_dump_subframe_lpc(miniflac_subframe_lpc* lpc, uint8_t indent);

void
miniflac_dump_residual(miniflac_residual* residual, uint8_t indent);

void
miniflac_dump_metadata(miniflac_metadata_t* metadata, uint8_t indent);

void
miniflac_dump_metadata_header(miniflac_metadata_header* header, uint8_t indent);

void
miniflac_dump_ogg(miniflac_ogg_t* ogg, uint8_t indent);

void
miniflac_dump_oggheader(miniflac_oggheader_t* oggheader, uint8_t indent);

void
miniflac_dump_streaminfo(miniflac_streaminfo_private_t* streaminfo, uint8_t indent);

void
miniflac_dump_streammarker(miniflac_streammarker_t* streammarker, uint8_t indent);

void
miniflac_dump_flac(miniflac_t* pFlac, uint8_t indent);

#ifdef __cplusplus
}
#endif

#endif
