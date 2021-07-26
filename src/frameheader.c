/* SPDX-License-Identifier: 0BSD */
#include "frameheader.h"

MINIFLAC_PRIVATE
void
miniflac_frame_header_init(miniflac_frame_header* header) {
    header->block_size_raw = 0;
    header->sample_rate_raw = 0;
    header->channel_assignment_raw = 0;
    header->sample_rate = 0;
    header->blocking_strategy = 0;
    header->block_size = 0;
    header->sample_rate = 0;
    header->channel_assignment = MINIFLAC_CHASSGN_NONE;
    header->channels = 0;
    header->bps = 0;
    header->sample_number = 0;
    header->crc8 = 0;
    header->state = MINIFLAC_FRAME_HEADER_SYNC;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_frame_header_decode(miniflac_frame_header* header, miniflac_bitreader_t* br) {
    uint64_t t;

    switch(header->state) {
        case MINIFLAC_FRAME_HEADER_SYNC: {
            if(miniflac_bitreader_fill(br,14)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,14);
            if(t != 0x3FFE) {
                miniflac_abort();
                return MINIFLAC_FRAME_SYNCCODE_INVALID;
            }
            miniflac_frame_header_init(header);
            header->state = MINIFLAC_FRAME_HEADER_RESERVEBIT_1;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_RESERVEBIT_1: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,1);
            if(t != 0) {
                miniflac_abort();
                return MINIFLAC_FRAME_RESERVED_BIT1;
            }
            header->state = MINIFLAC_FRAME_HEADER_BLOCKINGSTRATEGY;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_BLOCKINGSTRATEGY: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,1);
            header->blocking_strategy = t;
            header->state = MINIFLAC_FRAME_HEADER_BLOCKSIZE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_BLOCKSIZE: {
            if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,4);
            header->block_size_raw = t;
            header->block_size = 0;
            switch(header->block_size_raw) {
                case 0: {
                    miniflac_abort();
                    return MINIFLAC_FRAME_RESERVED_BLOCKSIZE;
                }
                case 1: {
                    header->block_size = 192;
                    break;
                }
                case 2: {
                    header->block_size = 576;
                    break;
                }
                case 3: {
                    header->block_size = 1152;
                    break;
                }
                case 4: {
                    header->block_size = 2304;
                    break;
                }
                case 5: {
                    header->block_size = 4608;
                    break;
                }
                case 8: {
                    header->block_size = 256;
                    break;
                }
                case 9: {
                    header->block_size = 512;
                    break;
                }
                case 10: {
                    header->block_size = 1024;
                    break;
                }
                case 11: {
                    header->block_size = 2048;
                    break;
                }
                case 12: {
                    header->block_size = 4096;
                    break;
                }
                case 13: {
                    header->block_size = 8192;
                    break;
                }
                case 14: {
                    header->block_size = 16384;
                    break;
                }
                case 15: {
                    header->block_size = 32768;
                    break;
                }
                default: break;
            }
            header->state = MINIFLAC_FRAME_HEADER_SAMPLERATE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLERATE: {
            if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,4);
            header->sample_rate_raw = t;
            switch(header->sample_rate_raw) {
                case 0: {
                    header->sample_rate = 0;
                    break;
                }
                case 1: {
                    header->sample_rate = 88200;
                    break;
                }
                case 2: {
                    header->sample_rate = 176400;
                    break;
                }
                case 3: {
                    header->sample_rate = 192000;
                    break;
                }
                case 4: {
                    header->sample_rate = 8000;
                    break;
                }
                case 5: {
                    header->sample_rate = 16000;
                    break;
                }
                case 6: {
                    header->sample_rate = 22050;
                    break;
                }
                case 7: {
                    header->sample_rate = 24000;
                    break;
                }
                case 8: {
                    header->sample_rate = 32000;
                    break;
                }
                case 9: {
                    header->sample_rate = 44100;
                    break;
                }
                case 10: {
                    header->sample_rate = 48000;
                    break;
                }
                case 11: {
                    header->sample_rate = 96000;
                    break;
                }
                case 12: /* fall-through */
                case 13: /* fall-through */
                case 14: {
                    header->sample_rate = 0;
                    break;
                }
                case 15: {
                    miniflac_abort();
                    return MINIFLAC_FRAME_INVALID_SAMPLE_RATE;
                }
                default: break;
            }

            header->state = MINIFLAC_FRAME_HEADER_CHANNELASSIGNMENT;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_CHANNELASSIGNMENT: {
            if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,4);
            if(t > 10) {
                miniflac_abort();
                return MINIFLAC_FRAME_RESERVED_CHANNEL_ASSIGNMENT;
            }

            if(t < 8) {
                header->channels = t + 1;
                header->channel_assignment = MINIFLAC_CHASSGN_NONE;
            } else {
                switch(t) {
                    case 8: {
                        header->channel_assignment = MINIFLAC_CHASSGN_LEFT_SIDE; break;
                    }
                    case 9: {
                        header->channel_assignment = MINIFLAC_CHASSGN_RIGHT_SIDE; break;
                    }
                    case 10: {
                        header->channel_assignment = MINIFLAC_CHASSGN_MID_SIDE; break;
                    }
                    default: break;
                }
                header->channels = 2;
            }

            header->channel_assignment_raw = t;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLESIZE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLESIZE: {
            if(miniflac_bitreader_fill(br,3)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,3);
            switch(t) {
                case 0: header->bps = 0; break;
                case 1: header->bps = 8; break;
                case 2: header->bps = 12; break;
                case 3: {
                    miniflac_abort();
                    return MINIFLAC_FRAME_RESERVED_SAMPLE_SIZE;
                }
                case 4: header->bps = 16; break;
                case 5: header->bps = 20; break;
                case 6: header->bps = 24; break;
                case 7: {
                    miniflac_abort();
                    return MINIFLAC_FRAME_RESERVED_SAMPLE_SIZE;
                }
            }
            header->state = MINIFLAC_FRAME_HEADER_RESERVEBIT_2;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_RESERVEBIT_2: {
            if(miniflac_bitreader_fill(br,1)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,1);
            if(t != 0) {
                miniflac_abort();
                return MINIFLAC_FRAME_RESERVED_BIT2;
            }
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_1;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_1: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);

            if((t & 0x80) == 0x00) {
                header->sample_number = t;
                header->state = MINIFLAC_FRAME_HEADER_BLOCKSIZE_MAYBE;
                goto flac_frame_blocksize_maybe;
            }
            else if((t & 0xE0) == 0xC0) {
                header->sample_number = (t & 0x1F) << 6;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_7;
                goto flac_frame_samplenumber_7;
            } else if((t & 0xF0) == 0xE0) {
                header->sample_number = (t & 0x0F) << 12;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_6;
                goto flac_frame_samplenumber_6;
            } else if((t & 0xF8) == 0xF0) {
                header->sample_number = (t & 0x07) << 18;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_5;
                goto flac_frame_samplenumber_5;
            } else if((t & 0xFC) == 0xF8) {
                header->sample_number = (t & 0x03) << 24;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_4;
                goto flac_frame_samplenumber_4;
            } else if((t & 0xFF) == 0xFE) {
                header->sample_number = (t & 0x01) << 30;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_3;
                goto flac_frame_samplenumber_3;
            } else if((t & 0xFF) == 0xFF) {
                header->sample_number = 0;
                header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_2;
                goto flac_frame_samplenumber_2;
            }
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_2: {
            flac_frame_samplenumber_2:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 30;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_3;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_3: {
            flac_frame_samplenumber_3:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 24;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_4;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_4: {
            flac_frame_samplenumber_4:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 18;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_5;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_5: {
            flac_frame_samplenumber_5:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 12;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_6;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_6: {
            flac_frame_samplenumber_6:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F) << 6;
            header->state = MINIFLAC_FRAME_HEADER_SAMPLENUMBER_7;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLENUMBER_7: {
            flac_frame_samplenumber_7:
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            header->sample_number += (t & 0x3F);
            header->state = MINIFLAC_FRAME_HEADER_BLOCKSIZE_MAYBE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_BLOCKSIZE_MAYBE: {
            flac_frame_blocksize_maybe:
            switch(header->block_size_raw) {
                case 6: {
                    if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,8) + 1;
                    header->block_size = t;
                    break;
                }
                case 7: {
                    if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,16) + 1;
                    header->block_size = t;
                    break;
                }
                default: break;
            }
            header->state = MINIFLAC_FRAME_HEADER_SAMPLERATE_MAYBE;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_SAMPLERATE_MAYBE: {
            switch(header->sample_rate_raw) {
                case 12: {
                    if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,8);
                    header->sample_rate = t * 1000;
                    break;
                }
                case 13: {
                    if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,16);
                    header->sample_rate = t;
                    break;
                }
                case 14: {
                    if(miniflac_bitreader_fill(br,16)) return MINIFLAC_CONTINUE;
                    t = miniflac_bitreader_read(br,16);
                    header->sample_rate = t * 10;
                    break;
                }
                default: break;
            }

            /* grab crc8 from bitreader before fill */
            header->crc8 = br->crc8;
            header->state = MINIFLAC_FRAME_HEADER_CRC8;
        }
        /* fall-through */
        case MINIFLAC_FRAME_HEADER_CRC8: {
            if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,8);
            if(header->crc8 != t) {
                miniflac_abort();
                return MINIFLAC_FRAME_CRC8_INVALID;
            }
        }
        /* fall-through */
        default: break;
    }

    header->state = MINIFLAC_FRAME_HEADER_SYNC;
    return MINIFLAC_OK;
}

