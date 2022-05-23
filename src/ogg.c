#include "ogg.h"
#include "unpack.h"

MINIFLAC_PRIVATE
void
miniflac_ogg_init(miniflac_ogg_t* ogg) {
    ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_O;
    ogg->version = 0;
    ogg->headertype = 0;
    ogg->granulepos = 0;
    ogg->serialno = 0;
    ogg->pageno = 0;
    ogg->segments = 0;
    ogg->curseg = 0;
    ogg->length = 0;
    ogg->pos = 0;
    miniflac_bitreader_init(&ogg->br);
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_ogg_sync(miniflac_ogg_t* ogg,miniflac_bitreader_t* br) {
    unsigned char c;
    uint8_t buffer[8];

    switch(ogg->state) {
        case MINIFLAC_OGG_SKIP: /* fall-through */
        case MINIFLAC_OGG_DATA: {
            while(ogg->pos < ogg->length) {
                if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
                miniflac_bitreader_discard(br,8);
                ogg->pos++;
            }
            ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_O;
        }
        /* fall-through */
        case MINIFLAC_OGG_CAPTUREPATTERN_O: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            c = (unsigned char)miniflac_bitreader_read(br,8);
            if(c != 'O') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_G1;
        }
        /* fall-through */
        case MINIFLAC_OGG_CAPTUREPATTERN_G1: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            c = (unsigned char)miniflac_bitreader_read(br,8);
            if(c != 'g') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_G2;
        }
        /* fall-through */
        case MINIFLAC_OGG_CAPTUREPATTERN_G2: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            c = (unsigned char)miniflac_bitreader_read(br,8);
            if(c != 'g') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_CAPTUREPATTERN_S;
        }
        /* fall-through */
        case MINIFLAC_OGG_CAPTUREPATTERN_S: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            c = (unsigned char)miniflac_bitreader_read(br,8);
            if(c != 'S') {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_VERSION;
        }
        /* fall-through */
        case MINIFLAC_OGG_VERSION: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            ogg->version = (uint8_t)miniflac_bitreader_read(br,8);
            if(ogg->version != 0) {
                miniflac_abort();
                return MINIFLAC_ERROR;
            }
            ogg->state = MINIFLAC_OGG_HEADERTYPE;
        }
        /* fall-through */
        case MINIFLAC_OGG_HEADERTYPE: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            ogg->headertype = (uint8_t)miniflac_bitreader_read(br,8);
            ogg->state = MINIFLAC_OGG_GRANULEPOS;
        }
        /* fall-through */
        case MINIFLAC_OGG_GRANULEPOS: {
            if(miniflac_bitreader_fill_nocrc(br,64)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            buffer[4] = miniflac_bitreader_read(br,8);
            buffer[5] = miniflac_bitreader_read(br,8);
            buffer[6] = miniflac_bitreader_read(br,8);
            buffer[7] = miniflac_bitreader_read(br,8);
            ogg->granulepos = miniflac_unpack_int64le(buffer);
            ogg->state = MINIFLAC_OGG_SERIALNO;
        }
        /* fall-through */
        case MINIFLAC_OGG_SERIALNO: {
            if(miniflac_bitreader_fill_nocrc(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            ogg->serialno = miniflac_unpack_int32le(buffer);
            ogg->state = MINIFLAC_OGG_PAGENO;
        }
        /* fall-through */
        case MINIFLAC_OGG_PAGENO: {
            if(miniflac_bitreader_fill_nocrc(br,32)) return MINIFLAC_CONTINUE;
            buffer[0] = miniflac_bitreader_read(br,8);
            buffer[1] = miniflac_bitreader_read(br,8);
            buffer[2] = miniflac_bitreader_read(br,8);
            buffer[3] = miniflac_bitreader_read(br,8);
            ogg->pageno = miniflac_unpack_uint32le(buffer);
            ogg->state = MINIFLAC_OGG_CHECKSUM;
        }
        /* fall-through */
        case MINIFLAC_OGG_CHECKSUM: {
            if(miniflac_bitreader_fill_nocrc(br,32)) return MINIFLAC_CONTINUE;
            miniflac_bitreader_discard(br,32);
            ogg->state = MINIFLAC_OGG_PAGESEGMENTS;
        }
        /* fall-through */
        case MINIFLAC_OGG_PAGESEGMENTS: {
            if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
            ogg->segments = (uint8_t) miniflac_bitreader_read(br,8);
            ogg->curseg = 0;
            ogg->length = 0;
            ogg->state = MINIFLAC_OGG_SEGMENTTABLE;
        }
        /* fall-through */
        case MINIFLAC_OGG_SEGMENTTABLE: {
            while(ogg->curseg < ogg->segments) {
              if(miniflac_bitreader_fill_nocrc(br,8)) return MINIFLAC_CONTINUE;
              ogg->length += miniflac_bitreader_read(br,8);
              ogg->curseg++;
            }
            ogg->pos = 0;
            ogg->state = MINIFLAC_OGG_DATA;
            return MINIFLAC_OK;
        }
    }
    return MINIFLAC_ERROR;
}


