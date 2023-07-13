.PHONY: all clean

MISC_CFLAGS = -DMINIFLAC_ABORT_ON_ERROR

OPTIMIZE = -O3
LTO = -flto

CFLAGS = $(LTO) -Wall -Wextra -fPIC -pg -g $(OPTIMIZE) $(MISC_CFLAGS)
LDFLAGS = $(LTO) -pg

OBJS = \
  src/application.o \
  src/bitreader.o \
  src/cuesheet.o \
  src/flac.o \
  src/frame.o \
  src/frameheader.o \
  src/metadata.o \
  src/metadataheader.o \
  src/mflac.o \
  src/ogg.o \
  src/oggheader.o \
  src/padding.o \
  src/picture.o \
  src/residual.o \
  src/seektable.o \
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
  src/application.c \
  src/bitreader.c \
  src/cuesheet.c \
  src/flac.c \
  src/frame.c \
  src/frameheader.c \
  src/metadata.c \
  src/metadataheader.c \
  src/mflac.c \
  src/padding.c \
  src/ogg.c \
  src/oggheader.c \
  src/picture.c \
  src/residual.c \
  src/seektable.c \
  src/streaminfo.c \
  src/streammarker.c \
  src/subframe.c \
  src/subframe_constant.c \
  src/subframe_fixed.c \
  src/subframeheader.c \
  src/subframe_lpc.c \
  src/subframe_verbatim.c \
  src/unpack.c \
  src/vorbiscomment.c

HEADERS = \
  src/application.h \
  src/common.h \
  src/bitreader.h \
  src/cuesheet.h \
  src/flac.h \
  src/mflac.h \
  src/frame.h \
  src/frameheader.h \
  src/streammarker.h \
  src/metadataheader.h \
  src/streaminfo.h \
  src/metadata.h \
  src/miniflac.h \
  src/ogg.h \
  src/oggheader.h \
  src/padding.h \
  src/picture.h \
  src/residual.h \
  src/seektable.h \
  src/subframe_fixed.h \
  src/subframe_lpc.h \
  src/subframe_constant.h \
  src/subframe_verbatim.h \
  src/subframeheader.h \
  src/subframe.h \
  src/unpack.h \
  src/vorbiscomment.h

all: libminiflac.a libminiflac.so miniflac.h \
     examples/basic-decoder-mflac \
     examples/basic-remuxer \
     examples/basic-decoder examples/single-byte-decoder \
	 utils/strip-headers examples/get-sizes examples/null-decoder \
	 examples/benchmark examples/just-decode \
	 examples/just-decode-singlefile-0 \
	 examples/just-decode-singlefile-1 \
	 examples/just-decode-singlefile-2 \
	 examples/just-decode-singlefile-3

miniflac.h: $(SOURCES) $(HEADERS) utils/build.pl
	./utils/build.pl > miniflac.h

libminiflac.a: $(OBJS)
	$(AR) rcs $@ $^

libminiflac.so: $(OBJS)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

examples/get-sizes: examples/get-sizes.o src/debug.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

examples/benchmark.o: examples/benchmark.c miniflac.h
	$(CC) $(CFLAGS) $(shell pkg-config --cflags flac) -c -o $@ $<

examples/basic-decoder.o: examples/basic-decoder.c miniflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/basic-remuxer.o: examples/basic-remuxer.c miniflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/basic-decoder-mflac.o: examples/basic-decoder-mflac.c miniflac.h mflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/null-decoder.o: examples/null-decoder.c miniflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/slurp.o: examples/slurp.c
	$(CC) $(CFLAGS) -c -o $@ $<

examples/just-decode.o: examples/just-decode.c
	$(CC) $(CFLAGS) -c -o $@ $<

examples/just-decode-singlefile-0.o: examples/just-decode-singlefile.c miniflac.h
	$(CC) -Wall -Wextra $(LTO) -pg -g -fPIC -O0 -c -o $@ $<

examples/just-decode-singlefile-1.o: examples/just-decode-singlefile.c miniflac.h
	$(CC) -Wall -Wextra $(LTO) -pg -g -fPIC -O1 -c -o $@ $<

examples/just-decode-singlefile-2.o: examples/just-decode-singlefile.c miniflac.h
	$(CC) -Wall -Wextra $(LTO) -pg -g -fPIC -O2 -c -o $@ $<

examples/just-decode-singlefile-3.o: examples/just-decode-singlefile.c miniflac.h
	$(CC) -Wall -Wextra $(LTO) -pg -g -fPIC -O3 -c -o $@ $<

examples/single-byte-decoder.o: examples/single-byte-decoder.c miniflac.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/basic-decoder: examples/basic-decoder.o examples/wav.o examples/pack.o examples/slurp.o src/debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

examples/basic-remuxer: examples/basic-remuxer.o examples/slurp.o src/debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

examples/basic-decoder-mflac: examples/basic-decoder-mflac.o examples/wav.o examples/pack.o src/debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

examples/benchmark: examples/benchmark.o examples/slurp.o examples/tictoc.o
	$(CC) -o $@ $^ $(LDFLAGS) -lrt $(shell pkg-config --libs flac)

examples/just-decode: examples/just-decode.o examples/slurp.o examples/tictoc.o libminiflac.a
	$(CC) -o $@ $^ $(LDFLAGS)

examples/just-decode-singlefile-0: examples/just-decode-singlefile-0.o examples/slurp.o examples/tictoc.o
	$(CC) -o $@ $^ $(LTO) -pg

examples/just-decode-singlefile-1: examples/just-decode-singlefile-1.o examples/slurp.o examples/tictoc.o
	$(CC) -o $@ $^ $(LTO) -pg

examples/just-decode-singlefile-2: examples/just-decode-singlefile-2.o examples/slurp.o examples/tictoc.o
	$(CC) -o $@ $^ $(LTO) -pg

examples/just-decode-singlefile-3: examples/just-decode-singlefile-3.o examples/slurp.o examples/tictoc.o
	$(CC) -o $@ $^ $(LTO) -pg

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
	rm -f examples/benchmark examples/benchmark.exe examples/benchmark.o
	rm -f examples/just-decode examples/just-decode.exe examples/just-decode.o
	rm -f examples/just-decode-singlefile examples/just-decode-singlefile.exe examples/just-decode-singlefile.o
	rm -f examples/just-decode-singlefile-0 examples/just-decode-singlefile-0.exe examples/just-decode-singlefile-0.o
	rm -f examples/just-decode-singlefile-1 examples/just-decode-singlefile-1.exe examples/just-decode-singlefile-1.o
	rm -f examples/just-decode-singlefile-2 examples/just-decode-singlefile-2.exe examples/just-decode-singlefile-2.o
	rm -f examples/just-decode-singlefile-3 examples/just-decode-singlefile-3.exe examples/just-decode-singlefile-3.o
	rm -f examples/wav.o examples/pack.o examples/slurp.o
	rm -f utils/strip-headers utils/strip-headers.exe utils/strip-headers.o
	rm -f src/debug.o
	rm -f examples/tictoc.o
