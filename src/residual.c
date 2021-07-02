/* SPDX-License-Identifier: 0BSD */
#include "residual.h"
#include <stddef.h>

static const uint8_t escape_codes[2] = {
    15,
    31,
};

MINIFLAC_PRIVATE
void
miniflac_residual_init(miniflac_residual* residual) {
    residual->coding_method = 0;
    residual->partition_order = 0;
    residual->rice_parameter = 0;
    residual->rice_size = 0;
    residual->msb = 0;
    residual->rice_parameter_size = 0;
    residual->value = 0;
    residual->partition = 0;
    residual->partition_total = 0;
    residual->residual = 0;
    residual->residual_total = 0;
    residual->state = MINIFLAC_RESIDUAL_CODING_METHOD;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_residual_decode(miniflac_residual* residual, miniflac_bitreader* br, uint32_t* pos, uint32_t block_size, uint8_t predictor_order, int32_t *output) {
    uint64_t temp;
    uint32_t temp_32;

    switch(residual->state) {
        case MINIFLAC_RESIDUAL_CODING_METHOD: {
            if(miniflac_bitreader_fill(br,2)) return MINIFLAC_CONTINUE;
            temp = miniflac_bitreader_read(br,2);
            if(temp > 1) {
                miniflac_abort();
                return MINIFLAC_RESERVED_CODING_METHOD;
            }
            residual->coding_method = temp;
            switch(residual->coding_method) {
                case 0: {
                    residual->rice_parameter_size = 4;
                    break;
                }
                case 1: {
                    residual->rice_parameter_size = 5;
                    break;
                }
            }
            residual->msb = 0;
            residual->state = MINIFLAC_RESIDUAL_PARTITION_ORDER;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_PARTITION_ORDER: {
            if(miniflac_bitreader_fill(br,4)) return MINIFLAC_CONTINUE;
            residual->partition_order = miniflac_bitreader_read(br,4);
            residual->partition_total = 1 << residual->partition_order;
            residual->state = MINIFLAC_RESIDUAL_RICE_PARAMETER;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_RICE_PARAMETER: {
            miniflac_residual_rice_parameter:
            if(miniflac_bitreader_fill(br,residual->rice_parameter_size)) return MINIFLAC_CONTINUE;
            residual->rice_parameter = miniflac_bitreader_read(br,residual->rice_parameter_size);

            residual->residual = 0;
            residual->residual_total = block_size >> residual->partition_order;
            if(residual->partition == 0) {
                residual->residual_total -= predictor_order;
            }

            if(residual->rice_parameter == escape_codes[residual->coding_method]) {
                residual->state = MINIFLAC_RESIDUAL_RICE_SIZE;
                goto miniflac_residual_rice_size;
            }
            residual->state = MINIFLAC_RESIDUAL_MSB;
            goto miniflac_residual_msb;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_RICE_SIZE: {
            miniflac_residual_rice_size:
            if(miniflac_bitreader_fill(br,5)) return MINIFLAC_CONTINUE;
            residual->rice_size = miniflac_bitreader_read(br,5);
            residual->state = MINIFLAC_RESIDUAL_RICE_VALUE;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_RICE_VALUE: {
            miniflac_residual_rice_value:
            if(miniflac_bitreader_fill(br,residual->rice_size)) return MINIFLAC_CONTINUE;
            residual->value = miniflac_bitreader_read_signed(br,residual->rice_size);
            if(output != NULL) {
                output[*pos] = residual->value;
            }
            *pos += 1;
            residual->residual++;
            if(residual->residual < residual->residual_total) {
                residual->state = MINIFLAC_RESIDUAL_RICE_VALUE;
                goto miniflac_residual_rice_value;
            }
            goto miniflac_residual_nextpart;
        }
        /* fall-through */
        case MINIFLAC_RESIDUAL_MSB: {
            miniflac_residual_msb:
            while(!miniflac_bitreader_fill(br,1)) {
                if(miniflac_bitreader_read(br,1)) {
                    residual->state = MINIFLAC_RESIDUAL_LSB;
                    goto miniflac_residual_lsb;
                }
                residual->msb++;
            }
            return MINIFLAC_CONTINUE;
        }
        case MINIFLAC_RESIDUAL_LSB: {
            miniflac_residual_lsb:
            if(miniflac_bitreader_fill(br,residual->rice_parameter)) return MINIFLAC_CONTINUE;
            temp_32 = (residual->msb << residual->rice_parameter) | ((uint32_t)miniflac_bitreader_read(br,residual->rice_parameter));
            residual->value = (temp_32 >> 1) ^ -(temp_32 & 1);

            if(output != NULL) {
                output[*pos] = residual->value;
            }
            *pos += 1;

            residual->msb = 0;
            residual->residual++;
            if(residual->residual < residual->residual_total) {
                residual->state = MINIFLAC_RESIDUAL_MSB;
                goto miniflac_residual_msb;
            }

            miniflac_residual_nextpart:
            residual->residual = 0;
            residual->partition++;
            if(residual->partition < residual->partition_total) {
                residual->state = MINIFLAC_RESIDUAL_RICE_PARAMETER;
                goto miniflac_residual_rice_parameter;
            }
            break;
        }
        default: break;
    }

    miniflac_residual_init(residual);
    return MINIFLAC_OK;
}


