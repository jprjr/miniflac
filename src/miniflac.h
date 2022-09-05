/* SPDX-License-Identifier: 0BSD
Copyright (C) 2022 John Regan <john@jrjrtech.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/
#ifndef MINIFLAC_H
#define MINIFLAC_H

/*

Building
========

In one C file, define MINIFLAC_IMPLEMENTATION before including this header, like:
#define MINIFLAC_IMPLEMENTATION
#include "miniflac.h"

Usage
=====

You'll want to allocate a decoder, initialize it with miniflac_init, then start
pushing data into it with functions like miniflac_sync or miniflac_decode.

All the "feeding" functions take the same first 4 parameters:
  1. miniflac* pFlac - a pointer to a miniflac struct.
  2. const uint8_t* data - a pointer to a stream of bytes to push.
  3. uint32_t length - the length of your stream of bytes.
  4. uint32_t* out_length - an out-variable, it will be updated with the
                            number of bytes consumed.
You'll to save any un-consumed bytes for your next call.

All the "feeding" / public functions return a MINIFLAC_RESULT enum, these are the
important values:
  < 0 : error
  0 : more data is required (MINIFLAC_CONTINUE)
  1 : success (MINIFLAC_OK)

You can use miniflac_sync to sync to a block boundary. It will automatically parse
a metadata block header or frame header, so you can inspect it and check on things
like the block size, bits-per-sample, etc. See the various struct definitions down
below. The example program in "examples/basic-decoder.c" does this. Keep calling
it until you get something other than MINIFLAC_CONTINUE (0). MINIFLAC_OK (1) means
you've found a block boundary. Anything < 0 is an error.

You can look at the .state field in the miniflac struct to see if you're in a
metadata block or audio frame, and proceed accordingly.

Decoding audio is done with miniflac_decode. You don't *have* to use miniflac_sync first,
you could just call miniflac_decode (this is done in the example program
"examples/single-byte-decoder.c"). The advantage of miniflac_sync is you can
look at the frame properties in the header before you start decoding.

miniflac_decode behaves similarly to miniflac_sync - when it returns MINIFLAC_OK (1),
your output buffer will have decoded audio samples in it. You'll be at the end
of the audio frame, so you can either call miniflac_sync to check the header of the next
frame, or call miniflac_decode to continue on.

*/

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#if !defined(MINIFLAC_API)
    #ifdef MINIFLAC_DLL
        #ifdef _WIN32
            #define MINIFLAC_DLL_IMPORT  __declspec(dllimport)
            #define MINIFLAC_DLL_EXPORT  __declspec(dllexport)
            #define MINIFLAC_DLL_PRIVATE static
        #else
            #if defined(__GNUC__) && __GNUC__ >= 4
                #define MINIFLAC_DLL_IMPORT  __attribute__((visibility("default")))
                #define MINIFLAC_DLL_EXPORT  __attribute__((visibility("default")))
                #define MINIFLAC_DLL_PRIVATE __attribute__((visibility("hidden")))
            #else
                #define MINIFLAC_DLL_IMPORT
                #define MINIFLAC_DLL_EXPORT
                #define MINIFLAC_DLL_PRIVATE static
            #endif
        #endif

        #ifdef MINIFLAC_IMPLEMENTATION
            #define MINIFLAC_API  MINIFLAC_DLL_EXPORT
        #else
            #define MINIFLAC_API  MINIFLAC_DLL_IMPORT
        #endif
        #define MINIFLAC_PRIVATE MINIFLAC_DLL_PRIVATE
    #else
        #define MINIFLAC_API extern
        #define MINIFLAC_PRIVATE static
    #endif
#endif

#if defined(__GNUC__) && __GNUC__ >= 2 && __GNUC_MINOR__ >= 5
#define MINIFLAC_PURE __attribute__((const))
#endif

#define MINIFLAC_APPLICATION_H
#define MINIFLAC_COMMON_H
#define MINIFLAC_BITREADER_H
#define MINIFLAC_CUESHEET_H
#define MINIFLAC_FRAME_H
#define MINIFLAC_FRAMEHEADER_H
#define MINIFLAC_METADATA_H
#define MINIFLAC_METADATA_HEADER_H
#define MINIFLAC_OGG_H
#define MINIFLAC_OGGHEADER_H
#define MINIFLAC_PADDING_H
#define MINIFLAC_PICTURE_H
#define MINIFLAC_RESIDUAL_H
#define MINIFLAC_SEEKTABLE_H
#define MINIFLAC_STREAMINFO_H
#define MINIFLAC_STREAMMARKER_H
#define MINIFLAC_SUBFRAME_CONSTANT_H
#define MINIFLAC_SUBFRAME_FIXED_H
#define MINIFLAC_SUBFRAME_H
#define MINIFLAC_SUBFRAME_HEADER_H
#define MINIFLAC_SUBFRAME_LPC_H
#define MINIFLAC_SUBFRAME_VERBATIM_H
#define MINIFLAC_UNPACK_H
#define MINIFLAC_VORBIS_COMMENT_H

#ifndef MINIFLAC_PURE
#define MINIFLAC_PURE
#endif

#inject typedef_structs

#inject typedef_enums

#inject enums

#inject structs

#ifdef __cplusplus
extern "C" {
#endif

#inject public_function_declarations

#ifdef __cplusplus
}
#endif

#endif

#ifdef MINIFLAC_IMPLEMENTATION

#ifdef MINIFLAC_ABORT_ON_ERROR
#include <stdlib.h>
#define miniflac_abort() abort()
#else
#define miniflac_abort()
#endif

#inject private_function_declarations

#inject code

#endif
