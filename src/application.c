#include "application.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_application_init(miniflac_application_t* application) {
    application->state = MINIFLAC_APPLICATION_ID;
    application->len = 0;
    application->pos = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_application_read_id(miniflac_application_t* application, miniflac_bitreader_t* br, uint32_t* id) {
    uint32_t t;
    switch(application->state) {
        case MINIFLAC_APPLICATION_ID: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            t = (uint32_t)miniflac_bitreader_read(br,32);
            if(id != NULL) {
                *id = t;
            }
            application->state = MINIFLAC_APPLICATION_DATA;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}


MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_application_read_data(miniflac_application_t* application, miniflac_bitreader_t* br, uint8_t* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint8_t d;
    switch(application->state) {
        case MINIFLAC_APPLICATION_ID: {
            r = miniflac_application_read_id(application,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_APPLICATION_DATA: {
            while(application->pos < application->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                d = (uint8_t) miniflac_bitreader_read(br,8);
                if(output != NULL && application->pos < length) {
                    output[application->pos] = d;
                }
                application->pos++;
            }
            if(outlen != NULL) {
                *outlen = application->len <= length ? application->len : length;
            }
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

