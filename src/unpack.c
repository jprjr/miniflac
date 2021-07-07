#include "unpack.h"
MINIFLAC_PRIVATE
uint32_t
miniflac_unpack_uint32le(uint8_t buffer[4]) {
    return (
      (((uint32_t)buffer[0]) << 0 ) |
      (((uint32_t)buffer[1]) << 8 ) |
      (((uint32_t)buffer[2]) << 16) |
      (((uint32_t)buffer[3]) << 24));
}

MINIFLAC_PRIVATE
int32_t
miniflac_unpack_int32le(uint8_t buffer[4]) {
    return (int32_t)miniflac_unpack_uint32le(buffer);
}

MINIFLAC_PRIVATE
uint64_t
miniflac_unpack_uint64le(uint8_t buffer[8]) {
    return (
      (((uint64_t)buffer[0]) << 0 ) |
      (((uint64_t)buffer[1]) << 8 ) |
      (((uint64_t)buffer[2]) << 16) |
      (((uint64_t)buffer[3]) << 24) |
      (((uint64_t)buffer[4]) << 32) |
      (((uint64_t)buffer[5]) << 40) |
      (((uint64_t)buffer[6]) << 48) |
      (((uint64_t)buffer[7]) << 56));
}

MINIFLAC_PRIVATE
int64_t
miniflac_unpack_int64le(uint8_t buffer[8]) {
    return (int64_t)miniflac_unpack_uint64le(buffer);
}

