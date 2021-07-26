#include "picture.h"
#include <stddef.h>

MINIFLAC_PRIVATE
void
miniflac_picture_init(miniflac_picture_t* picture) {
    picture->state = MINIFLAC_PICTURE_TYPE;
    picture->len = 0;
    picture->pos = 0;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_type(miniflac_picture_t* picture, miniflac_bitreader *br, uint32_t* type) {
    uint32_t t = 0;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,32);
            if(type != NULL) *type = t;
            picture->state = MINIFLAC_PICTURE_MIME_LENGTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_mime_length(miniflac_picture_t* picture, miniflac_bitreader *br, uint32_t* length) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: {
            r = miniflac_picture_read_type(picture,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            picture->len = miniflac_bitreader_read(br,32);
            picture->pos = 0;
            if(length != NULL) *length = picture->len;
            picture->state = MINIFLAC_PICTURE_MIME_STRING;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_mime_string(miniflac_picture_t* picture, miniflac_bitreader *br, char* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    char c;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: {
            r = miniflac_picture_read_mime_length(picture,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_MIME_STRING: {
            while(picture->pos < picture->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && picture->pos < length) {
                    output[picture->pos] = c;
                }
                picture->pos++;
            }
            if(outlen != NULL) {
                *outlen = picture->len <= length ? picture->len : length;
            }
            picture->state = MINIFLAC_PICTURE_DESCRIPTION_LENGTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_description_length(miniflac_picture_t* picture, miniflac_bitreader *br, uint32_t* length) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_MIME_STRING: {
            r = miniflac_picture_read_mime_string(picture,br,NULL,0,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_LENGTH: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            picture->len = miniflac_bitreader_read(br,32);
            picture->pos = 0;
            if(length != NULL) *length = picture->len;
            picture->state = MINIFLAC_PICTURE_DESCRIPTION_STRING;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_description_string(miniflac_picture_t* picture, miniflac_bitreader *br, char* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    char c;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_MIME_STRING: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_LENGTH: {
            r = miniflac_picture_read_description_length(picture,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_STRING: {
            while(picture->pos < picture->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (char)miniflac_bitreader_read(br,8);
                if(output != NULL && picture->pos < length) {
                    output[picture->pos] = c;
                }
                picture->pos++;
            }
            if(outlen != NULL) {
                *outlen = picture->len <= length ? picture->len : length;
            }
            picture->state = MINIFLAC_PICTURE_WIDTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_width(miniflac_picture_t* picture, miniflac_bitreader *br, uint32_t* width) {
    uint32_t t = 0;
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_MIME_STRING: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_STRING: {
            r = miniflac_picture_read_description_string(picture,br,NULL,0,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_WIDTH: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,32);
            if(width != NULL) *width = t;
            picture->state = MINIFLAC_PICTURE_HEIGHT;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_height(miniflac_picture_t* picture, miniflac_bitreader *br, uint32_t* height) {
    uint32_t t = 0;
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_MIME_STRING: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_STRING: /* fall-through */
        case MINIFLAC_PICTURE_WIDTH: {
            r = miniflac_picture_read_width(picture,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_HEIGHT: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,32);
            if(height != NULL) *height = t;
            picture->state = MINIFLAC_PICTURE_COLORDEPTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_colordepth(miniflac_picture_t* picture, miniflac_bitreader *br, uint32_t* colordepth) {
    uint32_t t = 0;
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_MIME_STRING: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_STRING: /* fall-through */
        case MINIFLAC_PICTURE_WIDTH: /* fall-through */
        case MINIFLAC_PICTURE_HEIGHT: {
            r = miniflac_picture_read_height(picture,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_COLORDEPTH: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,32);
            if(colordepth != NULL) *colordepth = t;
            picture->state = MINIFLAC_PICTURE_TOTALCOLORS;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_totalcolors(miniflac_picture_t* picture, miniflac_bitreader *br, uint32_t* totalcolors) {
    uint32_t t = 0;
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_MIME_STRING: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_STRING: /* fall-through */
        case MINIFLAC_PICTURE_WIDTH: /* fall-through */
        case MINIFLAC_PICTURE_HEIGHT: /* fall-through */
        case MINIFLAC_PICTURE_COLORDEPTH: {
            r = miniflac_picture_read_colordepth(picture,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_TOTALCOLORS: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            t = miniflac_bitreader_read(br,32);
            if(totalcolors != NULL) *totalcolors = t;
            picture->state = MINIFLAC_PICTURE_PICTURE_LENGTH;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_picture_length(miniflac_picture_t* picture, miniflac_bitreader *br, uint32_t* length) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_MIME_STRING: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_STRING: /* fall-through */
        case MINIFLAC_PICTURE_WIDTH: /* fall-through */
        case MINIFLAC_PICTURE_HEIGHT: /* fall-through */
        case MINIFLAC_PICTURE_COLORDEPTH: /* fall-through */
        case MINIFLAC_PICTURE_TOTALCOLORS: {
            r = miniflac_picture_read_totalcolors(picture,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_PICTURE_LENGTH: {
            if(miniflac_bitreader_fill(br,32)) return MINIFLAC_CONTINUE;
            picture->len = miniflac_bitreader_read(br,32);
            picture->pos = 0;
            if(length != NULL) *length = picture->len;
            picture->state = MINIFLAC_PICTURE_PICTURE_DATA;
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_picture_read_picture_data(miniflac_picture_t* picture, miniflac_bitreader *br, uint8_t* output, uint32_t length, uint32_t* outlen) {
    MINIFLAC_RESULT r = MINIFLAC_ERROR;
    uint8_t c;
    switch(picture->state) {
        case MINIFLAC_PICTURE_TYPE: /* fall-through */
        case MINIFLAC_PICTURE_MIME_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_MIME_STRING: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_LENGTH: /* fall-through */
        case MINIFLAC_PICTURE_DESCRIPTION_STRING: /* fall-through */
        case MINIFLAC_PICTURE_WIDTH: /* fall-through */
        case MINIFLAC_PICTURE_HEIGHT: /* fall-through */
        case MINIFLAC_PICTURE_COLORDEPTH: /* fall-through */
        case MINIFLAC_PICTURE_TOTALCOLORS: /* fall-through */
        case MINIFLAC_PICTURE_PICTURE_LENGTH: {
            r = miniflac_picture_read_picture_length(picture,br,NULL);
            if(r != MINIFLAC_OK) return r;
        }
        /* fall-through */
        case MINIFLAC_PICTURE_PICTURE_DATA: {
            if(picture->pos == picture->len) return MINIFLAC_METADATA_END;
            while(picture->pos < picture->len) {
                if(miniflac_bitreader_fill(br,8)) return MINIFLAC_CONTINUE;
                c = (uint8_t)miniflac_bitreader_read(br,8);
                if(output != NULL && picture->pos < length) {
                    output[picture->pos] = c;
                }
                picture->pos++;
            }
            if(outlen != NULL) {
                *outlen = picture->len <= length ? picture->len : length;
            }
            return MINIFLAC_OK;
        }
        default: break;
    }
    miniflac_abort();
    return MINIFLAC_ERROR;
}
