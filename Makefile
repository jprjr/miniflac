.PHONY: all clean

#SANITIZE_CFLAGS = -fsanitize=undefined
#SANITIZE_LDFLAGS = -fsanitize=undefined

OPTIMIZE = -O3

CFLAGS = $(SANITIZE_CFLAGS) -Wall -Wextra -fPIC -g $(OPTIMIZE) # -DMINIFLAC_ABORT_ON_ERROR
LDFLAGS = $(SANITIZE_LDFLAGS)

OBJS = \
  src/bitreader.o \
  src/flac.o \
  src/frame.o \
  src/frameheader.o \
  src/metadata.o \
  src/metadataheader.o \
  src/residual.o \
  src/subframe.o \
  src/subframe_constant.o \
  src/subframe_verbatim.o \
  src/subframe_fixed.o \
  src/subframe_lpc.o \
  src/subframeheader.o \
  src/streaminfo.o \
  src/streammarker.o

SOURCES = \
  src/flac.c \
  src/bitreader.c \
  src/frame.c \
  src/frameheader.c \
  src/metadata.c \
  src/metadataheader.c \
  src/residual.c \
  src/streaminfo.c \
  src/streammarker.c \
  src/subframe.c \
  src/subframe_constant.c \
  src/subframe_fixed.c \
  src/subframeheader.c \
  src/subframe_lpc.c \
  src/subframe_verbatim.c

HEADERS = \
  src/common.h \
  src/bitreader.h \
  src/streammarker.h \
  src/metadataheader.h \
  src/streaminfo.h \
  src/metadata.h \
  src/residual.h \
  src/subframe_fixed.h \
  src/subframe_lpc.h \
  src/subframe_constant.h \
  src/subframe_verbatim.h \
  src/subframeheader.h \
  src/subframe.h \
  src/frameheader.h \
  src/frame.h \
  src/flac.h

all: libminiflac.so miniflac.h examples/basic-decoder examples/single-byte-decoder utils/strip-headers examples/get-sizes

miniflac.h: $(SOURCES) $(HEADERS) utils/build.pl
	./utils/build.pl > miniflac.h

libminiflac.so: $(OBJS)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

examples/get-sizes: examples/get-sizes.o src/debug.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

examples/basic-decoder.o: examples/basic-decoder.c miniflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/single-byte-decoder.o: examples/single-byte-decoder.c miniflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/basic-decoder: examples/basic-decoder.o examples/wav.o examples/pack.o src/debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

examples/single-byte-decoder: examples/single-byte-decoder.o examples/wav.o examples/pack.o
	$(CC) -o $@ $^ $(LDFLAGS)

utils/strip-headers: utils/strip-headers.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f miniflac.h
	rm -f libminiflac.so $(OBJS)
	rm -f examples/basic-decoder examples/basic-decoder.exe examples/basic-decoder.o
	rm -f examples/single-byte-decoder examples/single-byte-decoder.exe examples/single-byte-decoder.o
	rm -f examples/get-sizes examples/get-sizes.exe examples/get-sizes.o
	rm -f examples/wav.o examples/pack.o
	rm -f utils/strip-headers utils/strip-headers.exe utils/strip-headers.o
	rm -f src/debug.o
