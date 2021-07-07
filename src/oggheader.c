#include "oggheader.h"

MINIFLAC_PRIVATE
void
miniflac_oggheader_init(miniflac_oggheader_t* oggheader) {
    oggheader->state = MINIFLAC_OGGHEADER_PACKETTYPE;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_oggheader_decode(miniflac_oggheader_t* oggheader, miniflac_bitreader* br) {
    switch(oggheader->state) {
        case MINIFLAC_OGGHEADER_PACKETTYPE: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((uint8_t)miniflac_bitreader_read(br,8) != 0x7F) {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_F;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_F: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((char)miniflac_bitreader_read(br,8) != 'F') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_L;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_L: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((char)miniflac_bitreader_read(br,8) != 'L') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_A;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_A: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((char)miniflac_bitreader_read(br,8) != 'A') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_C;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_C: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((char)miniflac_bitreader_read(br,8) != 'C') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_MAJOR;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_MAJOR: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((uint8_t)miniflac_bitreader_read(br,8) != 0x01) {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_MINOR;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_MINOR: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            if((uint8_t)miniflac_bitreader_read(br,8) != 0x00) {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            oggheader->state = MINIFLAC_OGGHEADER_HEADERPACKETS;
        }
        /* fall-through */
        case MINIFLAC_OGGHEADER_HEADERPACKETS: {
            if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
            miniflac_bitreader_discard(br,16);
            oggheader->state = MINIFLAC_OGGHEADER_PACKETTYPE;
        }
        /* fall-through */
        default: break;
    }
    return MINIFLAC_OK;
}

