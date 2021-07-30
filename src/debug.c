/* SPDX-License-Identifier: 0BSD */
#include "debug.h"
#include <stdio.h>

#define dump(indent,str) fprintf(stderr,"%*s" str, indent, "")
#define dumpf(indent,str,...) fprintf(stderr,"%*s" str, indent, "", __VA_ARGS__)

static const char* const miniflac_application_state_str[] = {
    "MINIFLAC_APPLICATION_ID",
    "MINIFLAC_APPLICATION_DATA",
};

static const char* const miniflac_cuesheet_state_str[] = {
    "MINIFLAC_CUESHEET_CATALOG",
    "MINIFLAC_CUESHEET_LEADIN",
    "MINIFLAC_CUESHEET_CDFLAG",
    "MINIFLAC_CUESHEET_SHEET_RESERVE",
    "MINIFLAC_CUESHEET_TRACKS",
    "MINIFLAC_CUESHEET_TRACKOFFSET",
    "MINIFLAC_CUESHEET_TRACKNUMBER",
    "MINIFLAC_CUESHEET_TRACKISRC",
    "MINIFLAC_CUESHEET_TRACKTYPE",
    "MINIFLAC_CUESHEET_TRACKPREEMPH",
    "MINIFLAC_CUESHEET_TRACK_RESERVE",
    "MINIFLAC_CUESHEET_TRACKPOINTS",
    "MINIFLAC_CUESHEET_INDEX_OFFSET",
    "MINIFLAC_CUESHEET_INDEX_NUMBER",
    "MINIFLAC_CUESHEET_INDEX_RESERVE",
};

static const char* const miniflac_picture_state_str[] = {
    "MINIFLAC_PICTURE_TYPE",
    "MINIFLAC_PICTURE_MIME_LENGTH",
    "MINIFLAC_PICTURE_MIME_STRING",
    "MINIFLAC_PICTURE_DESCRIPTION_LENGTH",
    "MINIFLAC_PICTURE_DESCRIPTION_STRING",
    "MINIFLAC_PICTURE_WIDTH",
    "MINIFLAC_PICTURE_HEIGHT",
    "MINIFLAC_PICTURE_COLORDEPTH",
    "MINIFLAC_PICTURE_TOTALCOLORS",
    "MINIFLAC_PICTURE_PICTURE_LENGTH",
    "MINIFLAC_PICTURE_PICTURE_DATA",
};

static const char* const miniflac_seektable_state_str[] = {
    "MINIFLAC_SEEKTABLE_SAMPLE_NUMBER",
    "MINIFLAC_SEEKTABLE_SAMPLE_OFFSET",
    "MINIFLAC_SEEKTABLE_SAMPLES",
};

static const char* const miniflac_vorbis_comment_state_str[] = {
    "MINIFLAC_VORBISCOMMENT_VENDOR_LENGTH",
    "MINIFLAC_VORBISCOMMENT_VENDOR_STRING",
    "MINIFLAC_VORBISCOMMENT_TOTAL_COMMENTS",
    "MINIFLAC_VORBISCOMMENT_COMMENT_LENGTH",
    "MINIFLAC_VORBISCOMMENT_COMMENT_STRING",
};

static const char* const miniflac_state_str[] = {
    "MINIFLAC_UNKNOWN",
    "MINIFLAC_STREAMMARKER",
    "MINIFLAC_METADATA_OR_FRAME",
    "MINIFLAC_METADATA",
    "MINIFLAC_FRAME",
};

static const char* const miniflac_frame_state_str[] = {
    "MINIFLAC_FRAME_HEADER",
    "MINIFLAC_FRAME_SUBFRAME",
    "MINIFLAC_FRAME_FOOTER",
};

static const char* const miniflac_ogg_state_str[] = {
    "MINIFLAC_OGG_CAPTUREPATTERN_O",
    "MINIFLAC_OGG_CAPTUREPATTERN_G1",
    "MINIFLAC_OGG_CAPTUREPATTERN_G2",
    "MINIFLAC_OGG_CAPTUREPATTERN_S",
    "MINIFLAC_OGG_VERSION",
    "MINIFLAC_OGG_HEADERTYPE",
    "MINIFLAC_OGG_GRANULEPOS",
    "MINIFLAC_OGG_SERIALNO",
    "MINIFLAC_OGG_PAGENO",
    "MINIFLAC_OGG_CHECKSUM",
    "MINIFLAC_OGG_PAGESEGMENTS",
    "MINIFLAC_OGG_SEGMENTTABLE",
    "MINIFLAC_OGG_DATA",
};

static const char* const miniflac_streammarker_state_str[] = {
    "MINIFLAC_STREAMMARKER_F",
    "MINIFLAC_STREAMMARKER_L",
    "MINIFLAC_STREAMMARKER_A",
    "MINIFLAC_STREAMMARKER_C",
};

static const char* const miniflac_oggheader_state_str[] = {
    "MINIFLAC_OGGHEADER_PACKETTYPE",
    "MINIFLAC_OGGHEADER_F",
    "MINIFLAC_OGGHEADER_L",
    "MINIFLAC_OGGHEADER_A",
    "MINIFLAC_OGGHEADER_C",
    "MINIFLAC_OGGHEADER_MAJOR",
    "MINIFLAC_OGGHEADER_MINOR",
    "MINIFLAC_OGGHEADER_HEADERPACKETS",
};

static const char* const miniflac_frame_header_state_str[] = {
    "MINIFLAC_FRAME_HEADER_SYNC",
    "MINIFLAC_FRAME_HEADER_RESERVEBIT_1",
    "MINIFLAC_FRAME_HEADER_BLOCKINGSTRATEGY",
    "MINIFLAC_FRAME_HEADER_BLOCKSIZE",
    "MINIFLAC_FRAME_HEADER_SAMPLERATE",
    "MINIFLAC_FRAME_HEADER_CHANNELASSIGNMENT",
    "MINIFLAC_FRAME_HEADER_SAMPLESIZE",
    "MINIFLAC_FRAME_HEADER_RESERVEBIT_2",
    "MINIFLAC_FRAME_HEADER_SAMPLENUMBER_1",
    "MINIFLAC_FRAME_HEADER_SAMPLENUMBER_2",
    "MINIFLAC_FRAME_HEADER_SAMPLENUMBER_3",
    "MINIFLAC_FRAME_HEADER_SAMPLENUMBER_4",
    "MINIFLAC_FRAME_HEADER_SAMPLENUMBER_5",
    "MINIFLAC_FRAME_HEADER_SAMPLENUMBER_6",
    "MINIFLAC_FRAME_HEADER_SAMPLENUMBER_7",
    "MINIFLAC_FRAME_HEADER_BLOCKSIZE_MAYBE",
    "MINIFLAC_FRAME_HEADER_SAMPLERATE_MAYBE",
    "MINIFLAC_FRAME_HEADER_CRC8",
};

static const char* const miniflac_chassgn_str[] = {
    "MINIFLAC_CHASSGN_NONE",
    "MINIFLAC_CHASSGN_LEFT_SIDE",
    "MINIFLAC_CHASSGN_RIGHT_SIDE",
    "MINIFLAC_CHASSGN_MID_SIDE",
};

static const char* const miniflac_subframe_state_str[] = {
    "MINIFLAC_SUBFRAME_HEADER",
    "MINIFLAC_SUBFRAME_CONSTANT",
    "MINIFLAC_SUBFRAME_VERBATIM",
    "MINIFLAC_SUBFRAME_FIXED",
    "MINIFLAC_SUBFRAME_LPC",
};

static const char* const miniflac_subframe_header_type_str[] = {
    "MINIFLAC_SUBFRAME_TYPE_UNKNOWN",
    "MINIFLAC_SUBFRAME_TYPE_CONSTANT",
    "MINIFLAC_SUBFRAME_TYPE_FIXED",
    "MINIFLAC_SUBFRAME_TYPE_LPC",
    "MINIFLAC_SUBFRAME_TYPE_VERBATIM",
};

static const char* const miniflac_metadata_type_str[] = {
    "MINIFLAC_METADATA_UNKNOWN",
    "MINIFLAC_METADATA_STREAMINFO",
    "MINIFLAC_METADATA_PADDING",
    "MINIFLAC_METADATA_APPLICATION"
    "MINIFLAC_METADATA_SEEKTABLE",
    "MINIFLAC_METADATA_VORBIS_COMMENT",
    "MINIFLAC_METADATA_CUESHEET",
    "MINIFLAC_METADATA_PICTURE",
    "MINIFLAC_METADATA_INVALID",
};

static const char* const miniflac_metadata_state_str[] = {
    "MINIFLAC_METADATA_HEADER",
    "MINIFLAC_METADATA_DATA",
};

static const char* const miniflac_subframe_header_state_str[] = {
    "MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1",
    "MINIFLAC_SUBFRAME_HEADER_KIND",
    "MINIFLAC_SUBFRAME_HEADER_WASTED_BITS",
    "MINIFLAC_SUBFRAME_HEADER_UNARY",
};

static const char* const miniflac_subframe_constant_state_str[] = {
    "MINIFLAC_SUBFRAME_CONSTANT_DECODE",
};

static const char* const miniflac_subframe_verbatim_state_str[] = {
    "MINIFLAC_SUBFRAME_VERBATIM_DECODE",
};

static const char* const miniflac_subframe_fixed_state_str[] = {
    "MINIFLAC_SUBFRAME_FIXED_DECODE",
};

static const char* const miniflac_subframe_lpc_state_str[] = {
    "MINIFLAC_SUBFRAME_LPC_PRECISION",
    "MINIFLAC_SUBFRAME_LPC_SHIFT",
    "MINIFLAC_SUBFRAME_LPC_COEFF",
};

static const char* const miniflac_residual_state_str[] = {
    "MINIFLAC_RESIDUAL_CODING_METHOD",
    "MINIFLAC_RESIDUAL_PARTITION_ORDER",
    "MINIFLAC_RESIDUAL_RICE_PARAMETER",
    "MINIFLAC_RESIDUAL_RICE_SIZE",
    "MINIFLAC_RESIDUAL_RICE_VALUE",
    "MINIFLAC_RESIDUAL_MSB",
    "MINIFLAC_RESIDUAL_LSB",
};

static const char* const miniflac_streaminfo_state_str[] = {
    "MINIFLAC_STREAMINFO_MINBLOCKSIZE",
    "MINIFLAC_STREAMINFO_MAXBLOCKSIZE",
    "MINIFLAC_STREAMINFO_MINFRAMESIZE",
    "MINIFLAC_STREAMINFO_MAXFRAMESIZE",
    "MINIFLAC_STREAMINFO_SAMPLERATE",
    "MINIFLAC_STREAMINFO_CHANNELS",
    "MINIFLAC_STREAMINFO_BPS",
    "MINIFLAC_STREAMINFO_TOTALSAMPLES",
    "MINIFLAC_STREAMINFO_MD5_1",
    "MINIFLAC_STREAMINFO_MD5_2",
    "MINIFLAC_STREAMINFO_MD5_3",
    "MINIFLAC_STREAMINFO_MD5_4",
};

void miniflac_dump_application(miniflac_application_t* application, uint8_t indent) {
    dumpf(indent,"application (%lu bytes):\n",sizeof(miniflac_application_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_application_state_str[application->state]);
    dumpf(indent,"len: %u\n",application->len);
    dumpf(indent,"pos: %u\n",application->pos);
}

void miniflac_dump_cuesheet(miniflac_cuesheet_t* cuesheet, uint8_t indent) {
    dumpf(indent,"cuesheet (%lu bytes):\n",sizeof(miniflac_cuesheet_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_cuesheet_state_str[cuesheet->state]);
    dumpf(indent,"pos: %u\n",cuesheet->pos);
    dumpf(indent,"track: %u\n",cuesheet->track);
    dumpf(indent,"tracks: %u\n",cuesheet->tracks);
    dumpf(indent,"point: %u\n",cuesheet->point);
    dumpf(indent,"points: %u\n",cuesheet->points);
}

void miniflac_dump_picture(miniflac_picture_t* picture, uint8_t indent) {
    dumpf(indent,"picture (%lu bytes):\n",sizeof(miniflac_picture_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_picture_state_str[picture->state]);
    dumpf(indent,"len: %u\n",picture->len);
    dumpf(indent,"pos: %u\n",picture->pos);
}

void miniflac_dump_seektable(miniflac_seektable_t* seektable, uint8_t indent) {
    dumpf(indent,"seektable (%lu bytes):\n",sizeof(miniflac_seektable_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_seektable_state_str[seektable->state]);
    dumpf(indent,"len: %u\n",seektable->len);
    dumpf(indent,"pos: %u\n",seektable->pos);
}


void miniflac_dump_vorbis_comment(miniflac_vorbis_comment_t* vorbis_comment, uint8_t indent) {
    dumpf(indent,"vorbis_comment (%lu bytes):\n",sizeof(miniflac_vorbis_comment_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_vorbis_comment_state_str[vorbis_comment->state]);
    dumpf(indent,"len: %u\n",vorbis_comment->len);
    dumpf(indent,"pos: %u\n",vorbis_comment->pos);
    dumpf(indent,"tot: %u\n",vorbis_comment->tot);
    dumpf(indent,"cur: %u\n",vorbis_comment->cur);
}

void
miniflac_dump_bitreader(miniflac_bitreader_t* br, uint8_t indent) {
    dumpf(indent,"bitreader (%lu bytes):\n",sizeof(miniflac_bitreader_t));
    indent+=2;
    dumpf(indent,"val: 0x%08lx\n",br->val);
    dumpf(indent,"bits: %u\n",br->bits);
    dumpf(indent,"crc8: %u\n",br->crc8);
    dumpf(indent,"crc16: %u\n",br->crc16);
    dumpf(indent,"pos: %u\n",br->pos);
    dumpf(indent,"len: %u\n",br->len);
}

void
miniflac_dump_frame_header(miniflac_frame_header_t* header, uint8_t indent) {
    dumpf(indent,"frame_header (%lu bytes):\n",sizeof(miniflac_frame_header_t));
    indent+=2;
    dumpf(indent,"state: %s\n",miniflac_frame_header_state_str[header->state]);
    dumpf(indent,"block_size_raw: %u\n",header->block_size_raw);
    dumpf(indent,"sample_rate_raw: %u\n",header->sample_rate_raw);
    dumpf(indent,"channel_assignment_raw: %u\n",header->channel_assignment_raw);
    dumpf(indent,"blocking_strategy: %u\n",header->blocking_strategy);
    dumpf(indent,"block_size: %u\n",header->block_size);
    dumpf(indent,"sample_rate: %u\n",header->sample_rate);
    dumpf(indent,"channel_assignment: %s\n",miniflac_chassgn_str[header->channel_assignment]);
    dumpf(indent,"channels: %u\n",header->channels);
    dumpf(indent,"bps: %u\n",header->bps);
    dumpf(indent,"sample_number: %lu\n",header->sample_number);
    dumpf(indent,"frame_number: %u\n",header->frame_number);
    dumpf(indent,"crc8: %u\n",header->crc8);
}

void
miniflac_dump_residual(miniflac_residual_t* residual, uint8_t indent) {
    dumpf(indent,"residual (%lu bytes):\n",sizeof(miniflac_residual_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_residual_state_str[residual->state]);
    dumpf(indent,"coding_method: %u\n",residual->coding_method);
    dumpf(indent,"partition_order: %u\n",residual->partition_order);
    dumpf(indent,"rice_parameter: %u\n",residual->rice_parameter);
    dumpf(indent,"rice_size: %u\n",residual->rice_size);
    dumpf(indent,"msb: %u\n",residual->msb);
    dumpf(indent,"rice_parameter_size: %u\n",residual->rice_parameter_size);
    dumpf(indent,"value: %d\n",residual->value);
    dumpf(indent,"partition: %u\n",residual->partition);
    dumpf(indent,"partition_total: %u\n",residual->partition_total);
    dumpf(indent,"residual: %u\n",residual->residual);
    dumpf(indent,"residual_total: %u\n",residual->residual_total);
}

void
miniflac_dump_subframe_header(miniflac_subframe_header_t* header, uint8_t indent) {
    dumpf(indent,"subframe_header (%lu bytes):\n",sizeof(miniflac_subframe_header_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_subframe_header_state_str[header->state]);
    dumpf(indent,"type: %s\n",miniflac_subframe_header_type_str[header->type]);
    dumpf(indent,"order: %u\n",header->order);
    dumpf(indent,"wasted_bits: %u\n",header->wasted_bits);
    dumpf(indent,"type_raw: %u\n",header->type_raw);
}

void
miniflac_dump_subframe_constant(miniflac_subframe_constant_t* c, uint8_t indent) {
    dumpf(indent,"subframe_constant (%lu bytes):\n",sizeof(miniflac_subframe_constant_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_subframe_constant_state_str[c->state]);
}

void
miniflac_dump_subframe_verbatim(miniflac_subframe_verbatim_t* c, uint8_t indent) {
    dumpf(indent,"subframe_verbatim (%lu bytes):\n",sizeof(miniflac_subframe_verbatim_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_subframe_verbatim_state_str[c->state]);
    dumpf(indent,"pos: %u\n",c->pos);
}

void
miniflac_dump_subframe_fixed(miniflac_subframe_fixed_t* c, uint8_t indent) {
    dumpf(indent,"subframe_fixed (%lu bytes):\n",sizeof(miniflac_subframe_fixed_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_subframe_fixed_state_str[c->state]);
    dumpf(indent,"pos: %u\n",c->pos);
    miniflac_dump_residual(&c->residual,indent);
}

void
miniflac_dump_subframe_lpc(miniflac_subframe_lpc_t* l, uint8_t indent) {
    uint32_t i;
    dumpf(indent,"subframe_lpc (%lu bytes):\n",sizeof(miniflac_subframe_lpc_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_subframe_lpc_state_str[l->state]);
    dumpf(indent,"pos: %u\n",l->pos);
    dumpf(indent,"precision: %u\n",l->precision);
    dumpf(indent,"shift: %u\n",l->shift);
    dump(indent,"coefficients:\n");
    for(i=0;i<32;i++) {
        dumpf(indent+1,"coefficients[%u]: %d\n",i,l->coefficients[i]);
    }
    miniflac_dump_residual(&l->residual,indent);
}

void
miniflac_dump_subframe(miniflac_subframe_t* subframe, uint8_t indent) {
    dumpf(indent,"subframe (%lu bytes):\n",sizeof(miniflac_subframe_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_subframe_state_str[subframe->state]);
    dumpf(indent,"bps: %u\n",subframe->bps);
    miniflac_dump_subframe_header(&subframe->header,indent);
    miniflac_dump_subframe_constant(&subframe->constant,indent);
    miniflac_dump_subframe_verbatim(&subframe->verbatim,indent);
    miniflac_dump_subframe_fixed(&subframe->fixed,indent);
    miniflac_dump_subframe_lpc(&subframe->lpc,indent);
}


void
miniflac_dump_frame(miniflac_frame_t* frame, uint8_t indent) {
    dumpf(indent,"frame (%lu bytes):\n",sizeof(miniflac_frame_t));
    indent +=2;
    dumpf(indent,"state: %s\n",miniflac_frame_state_str[frame->state]);
    dumpf(indent,"cur_subframe: %u\n",frame->cur_subframe);
    dumpf(indent,"crc16: %u\n",frame->crc16);
    miniflac_dump_frame_header(&frame->header,indent);
    miniflac_dump_subframe(&frame->subframe,indent);
}

void miniflac_dump_streaminfo(miniflac_streaminfo_t* streaminfo, uint8_t indent) {
    dumpf(indent,"streaminfo: (%lu bytes)\n",sizeof(miniflac_streaminfo_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_streaminfo_state_str[streaminfo->state]);
    dumpf(indent,"pos: %u\n",streaminfo->pos);
    dumpf(indent,"sample_rate: %u\n",streaminfo->sample_rate);
    dumpf(indent,"bps: %u\n",streaminfo->bps);
}

void miniflac_dump_metadata_header(miniflac_metadata_header_t* header, uint8_t indent) {
    dumpf(indent,"header (%lu bytes):\n",sizeof(miniflac_metadata_header_t));
    indent += 2;
    dumpf(indent,"is_last: %u\n",header->is_last);
    dumpf(indent,"type_raw: %u\n", header->type_raw);
    dumpf(indent,"type: %s \n", miniflac_metadata_type_str[header->type]);
    dumpf(indent,"length: %u\n",header->length);
}

void miniflac_dump_metadata(miniflac_metadata_t* metadata, uint8_t indent) {
    dumpf(indent,"metadata (%lu bytes):\n",sizeof(miniflac_metadata_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_metadata_state_str[metadata->state]);
    dumpf(indent,"pos: %u\n", metadata->pos);
    miniflac_dump_metadata_header(&metadata->header,indent);
    miniflac_dump_streaminfo(&metadata->streaminfo,indent);
    miniflac_dump_vorbis_comment(&metadata->vorbis_comment,indent);
    miniflac_dump_picture(&metadata->picture,indent);
    miniflac_dump_cuesheet(&metadata->cuesheet,indent);
    miniflac_dump_seektable(&metadata->seektable,indent);
    miniflac_dump_application(&metadata->application,indent);
}

void
miniflac_dump_streammarker(miniflac_streammarker_t* streammarker, uint8_t indent) {
    dumpf(indent,"streammarker (%lu bytes):\n",sizeof(miniflac_streammarker_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_streammarker_state_str[streammarker->state]);
}

void
miniflac_dump_oggheader(miniflac_oggheader_t* oggheader, uint8_t indent) {
    dumpf(indent,"header (%lu bytes):\n",sizeof(miniflac_oggheader_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_oggheader_state_str[oggheader->state]);
}

void
miniflac_dump_ogg(miniflac_ogg_t* ogg, uint8_t indent) {
    dumpf(indent,"ogg (%lu bytes):\n",sizeof(miniflac_ogg_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_ogg_state_str[ogg->state]);
    miniflac_dump_bitreader(&ogg->br,indent);
    dumpf(indent,"version: %u\n", ogg->version);
    dumpf(indent,"headertype: %u\n", ogg->headertype);
    dumpf(indent+2,"continuation: %d\n",(ogg->headertype & 0x01) == 0x01);
    dumpf(indent+2,"b_o_s: %d\n",(ogg->headertype & 0x02) == 0x02);
    dumpf(indent+2,"e_o_s: %d\n",(ogg->headertype & 0x04) == 0x04);
    dumpf(indent,"granulepos: %ld\n", ogg->granulepos);
    dumpf(indent,"serialno: %d\n", ogg->serialno);
    dumpf(indent,"pageno: %u\n", ogg->pageno);
    dumpf(indent,"segments: %u\n", ogg->segments);
    dumpf(indent,"curseg: %u\n", ogg->curseg);
    dumpf(indent,"length: %u\n", ogg->length);
    dumpf(indent,"pos: %u\n", ogg->pos);
}

void
miniflac_dump_flac(miniflac_t* pFlac,uint8_t indent) {
    dumpf(indent,"miniflac (%lu bytes):\n",sizeof(miniflac_t));
    indent += 2;
    dumpf(indent,"state: %s\n",miniflac_state_str[pFlac->state]);
    miniflac_dump_bitreader(&pFlac->br,indent);
    miniflac_dump_ogg(&pFlac->ogg,indent);
    miniflac_dump_oggheader(&pFlac->oggheader,indent);
    miniflac_dump_streammarker(&pFlac->streammarker,indent);
    miniflac_dump_metadata(&pFlac->metadata,indent);
    miniflac_dump_frame(&pFlac->frame,indent);
}
