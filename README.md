# miniflac

A single-file C library for decoding FLAC streams. Does not use any C library
functions, does not allocate any memory.

## Features

* push-style API
* does not allocate memory or use any c library functions
* handles native FLAC as well as Ogg FLAC
* single C file
* metadata decoding for:
  * [`STREAMINFO`](https://xiph.org/flac/format.html#metadata_block_streaminfo)
  * [`VORBIS_COMMENT`](https://xiph.org/flac/format.html#metadata_block_vorbis_comment)
  * [`PICTURE`](https://xiph.org/flac/format.html#metadata_block_picture)
  * [`CUESHEET`](https://xiph.org/flac/format.html#metadata_block_cuesheet)

## Future Improvements

* Speed optimizations
* Memory reduction
* Metadata Support for (in priority order):
  * [`SEEKTABLE`](https://xiph.org/flac/format.html#metadata_block_seektable)
  * [`APPLICATION`](https://xiph.org/flac/format.html#metadata_block_application)

## Usage

In one C file define `MINIFLAC_IMPLEMENTATION` before including `miniflac.h`.

Then allocate a `miniflac_t` struct, and call `miniflac_init` to initialize
the struct.

All functions follow a similar paradigm - you provide a buffer of data,
the length of the buffer, and an out-variable to record the amount of data
consumed. If more bytes are needed, functions return `MINIFLAC_CONTINUE`. You can
loop around these functions until they return something other than
`MINIFLAC_CONTINUE`.

`miniflac_sync` will read data until it parses a metadata header or audio
frame header. You can inspect the struct to determine if you're in a
metadata block or audio frame, and respond accordingly.

`miniflac_decode` will read data until it's decoded an audio frame. You can
check the size of the audio frame by inspecting the `frame.header` struct.

See the example programs under the `examples` directory.

## Details

`miniflac` offers a "push-style" interface. After initializing the decoder,
you give it a chunk of data. It will consume some amount of data (not
necessarily all) and decode audio. You'll need to keep track of how
much data was consumed, and hold on to any un-consumed bytes for
the next function call.

There's no callbacks involved, and you can provide as little as one byte
at a time.

This library does not perform any data allocations, everything can be
statically allocated if you like. A library user will need to provide
a buffer for writing decoded audio samples. FLAC supports up to 8 channels
of audio, and up to 65535 samples per frame of audio, so the maximum
required audio buffer is:

` (8 channels) * (65535 samples) * (4 bytes/sample) = 2097120 bytes`

In practice you often need less, most FLAC files are in the "streamable"
subset, which limits the frame size to 16384 samples, or 4608 samples if the
sample rate is <= 48kHz, and probably only 2 channels.

Some parts of this library are pretty un-optimized - like the states
for fixed decoding and lpc decoding each have their own residual
decoder, that could be shared instead.

## LICENSE

BSD Zero Clause (see the `LICENSE` file).
