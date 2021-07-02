/* SPDX-License-Identifier: 0BSD */
#include "metadataheader.h"

MINIFLAC_PRIVATE
void
miniflac_metadata_header_init(miniflac_metadata_header* header) {
    header->state = MINIFLAC_METADATA_LAST_FLAG;
    header->type = MINIFLAC_METADATA_UNKNOWN;
    header->is_last = 0;
    header->type_raw = 0;
    header->length = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_metadata_header_fill(miniflac_metadata_header* header, miniflac_bitreader *br) {
    switch(header->state) {
        case MINIFLAC_METADATA_LAST_FLAG: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            miniflac_metadata_header_init(header);
            header->is_last = (uint8_t)miniflac_bitreader_read(br,1);
            header->state = MINIFLAC_METADATA_BLOCK_TYPE;
        }
        /* fall-through */
        case MINIFLAC_METADATA_BLOCK_TYPE: {
            if(miniflac_bitreader_fill(br,7)) return MINIFLAC_CONTINUE;
            header->type_raw = (uint8_t)miniflac_bitreader_read(br,7);
            switch(header->type_raw) {
                case 0: header->type = MINIFLAC_METADATA_STREAMINFO; break;
                case 1: header->type = MINIFLAC_METADATA_PADDING; break;
                case 2: header->type = MINIFLAC_METADATA_APPLICATION; break;
                case 3: header->type = MINIFLAC_METADATA_SEEKTABLE; break;
                case 4: header->type = MINIFLAC_METADATA_VORBIS_COMMENT; break;
                case 5: header->type = MINIFLAC_METADATA_CUESHEET; break;
                case 6: header->type = MINIFLAC_METADATA_PICTURE; break;
                case 127: {
                    header->type = MINIFLAC_METADATA_INVALID;
                    miniflac_abort();
                    return MINIFLAC_METADATA_TYPE_INVALID;
                }
                default: {
                    header->type = MINIFLAC_METADATA_UNKNOWN;
                    miniflac_abort();
                    return MINIFLAC_METADATA_TYPE_RESERVED;
                }
            }
            header->state = MINIFLAC_METADATA_LENGTH;
        }
        /* fall-through */
        case MINIFLAC_METADATA_LENGTH: {
            if(miniflac_bitreader_fill(br,24)) return MINIFLAC_CONTINUE;
            header->length = (uint32_t) miniflac_bitreader_read(br,24);
            header->state = MINIFLAC_METADATA_LAST_FLAG;
            break;
        }
        default: break;
    }
    return MINIFLAC_OK;
}

