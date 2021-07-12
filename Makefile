.PHONY: all clean

SANITIZE_CFLAGS = -fsanitize=undefined
SANITIZE_LDFLAGS = -fsanitize=undefined

MISC_CFLAGS = -DMINIFLAC_ABORT_ON_ERROR

OPTIMIZE = -O3

CFLAGS = $(SANITIZE_CFLAGS) -Wall -Wextra -fPIC -g $(OPTIMIZE) $(MISC_CFLAGS)
LDFLAGS = $(SANITIZE_LDFLAGS)

OBJS = \
  src/bitreader.o \
  src/flac.o \
  src/frame.o \
  src/frameheader.o \
  src/metadata.o \
  src/metadataheader.o \
  src/ogg.o \
  src/oggheader.o \
  src/residual.o \
  src/subframe.o \
  src/subframe_constant.o \
  src/subframe_verbatim.o \
  src/subframe_fixed.o \
  src/subframe_lpc.o \
  src/subframeheader.o \
  src/streaminfo.o \
  src/streammarker.o \
  src/vorbiscomment.o \
  src/unpack.o

SOURCES = \
  src/bitreader.c \
  src/flac.c \
  src/frame.c \
  src/frameheader.c \
  src/metadata.c \
  src/metadataheader.c \
  src/ogg.c \
  src/oggheader.c \
  src/residual.c \
  src/streaminfo.c \
  src/streammarker.c \
  src/subframe.c \
  src/subframe_constant.c \
  src/subframe_fixed.c \
  src/subframeheader.c \
  src/subframe_lpc.c \
  src/subframe_verbatim.c \
  src/unpack.c

HEADERS = \
  src/common.h \
  src/bitreader.h \
  src/flac.h \
  src/frame.h \
  src/frameheader.h \
  src/streammarker.h \
  src/metadataheader.h \
  src/streaminfo.h \
  src/metadata.h \
  src/ogg.h \
  src/oggheader.h \
  src/residual.h \
  src/subframe_fixed.h \
  src/subframe_lpc.h \
  src/subframe_constant.h \
  src/subframe_verbatim.h \
  src/subframeheader.h \
  src/subframe.h \
  src/unpack.h

all: libminiflac.a libminiflac.so miniflac.h examples/basic-decoder examples/single-byte-decoder utils/strip-headers examples/get-sizes examples/null-decoder

miniflac.h: $(SOURCES) $(HEADERS) utils/build.pl
	./utils/build.pl > miniflac.h

libminiflac.a: $(OBJS)
	$(AR) rcs $@ $^

libminiflac.so: $(OBJS)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

examples/get-sizes: examples/get-sizes.o src/debug.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

examples/basic-decoder.o: examples/basic-decoder.c miniflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/null-decoder.o: examples/null-decoder.c miniflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/single-byte-decoder.o: examples/single-byte-decoder.c miniflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/basic-decoder: examples/basic-decoder.o examples/wav.o examples/pack.o examples/slurp.o src/debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

examples/null-decoder: examples/null-decoder.o src/debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

examples/single-byte-decoder: examples/single-byte-decoder.o examples/wav.o examples/pack.o
	$(CC) -o $@ $^ $(LDFLAGS)

utils/strip-headers: utils/strip-headers.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f miniflac.h
	rm -f libminiflac.a libminiflac.so $(OBJS)
	rm -f examples/basic-decoder examples/basic-decoder.exe examples/basic-decoder.o
	rm -f examples/single-byte-decoder examples/single-byte-decoder.exe examples/single-byte-decoder.o
	rm -f examples/get-sizes examples/get-sizes.exe examples/get-sizes.o
	rm -f examples/null-decoder examples/null-decoder.exe examples/null-decoder.o
	rm -f examples/wav.o examples/pack.o examples/slurp.o
	rm -f utils/strip-headers utils/strip-headers.exe utils/strip-headers.o
	rm -f src/debug.o
