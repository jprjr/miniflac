#!/usr/bin/env perl

# tool to join the header and sources files into the single-file build

use strict;
use warnings;
use Data::Dumper;

my @typedef_structs;
my @typedef_enums;
my @structs;
my @enums;
my @function_declarations;
my @private_function_declarations;
my @public_function_declarations;

my @code;

my @sources = qw[
src/flac.c
src/unpack.c
src/bitreader.c
src/oggheader.c
src/ogg.c
src/frame.c
src/frameheader.c
src/vorbiscomment.c
src/picture.c
src/cuesheet.c
src/metadata.c
src/metadataheader.c
src/residual.c
src/streaminfo.c
src/streammarker.c
src/subframe.c
src/subframe_constant.c
src/subframe_fixed.c
src/subframeheader.c
src/subframe_lpc.c
src/subframe_verbatim.c
];

my @headers = qw[
src/common.h
src/unpack.h
src/bitreader.h
src/oggheader.h
src/ogg.h
src/streammarker.h
src/metadataheader.h
src/streaminfo.h
src/vorbiscomment.h
src/picture.h
src/cuesheet.h
src/metadata.h
src/residual.h
src/subframe_fixed.h
src/subframe_lpc.h
src/subframe_constant.h
src/subframe_verbatim.h
src/subframeheader.h
src/subframe.h
src/frameheader.h
src/frame.h
src/flac.h
];

sub trim_end {
    my $s = shift;
    $s =~ s/\s+$//;
    return $s;
}

foreach my $source (@sources) {
    open(my $fh, '<', $source) or die "error opening $source: $!";
    my @lines = map { trim_end($_) } <$fh>;
    close($fh);

    for(my $i=$#lines; $i >= 0; $i--) {
        if(length($lines[$i]) == 0) {
            delete($lines[$i]);
        } else {
            last;
        }
    }

    @lines = reverse @lines;

    for(my $i=$#lines; $i >= 0; $i--) {
        if(length($lines[$i]) == 0) {
            delete($lines[$i]);
        } else {
            last;
        }
    }

    @lines = reverse @lines;

    @lines = grep { $_ !~ m/^\#include/ } @lines;
    @lines = grep { $_ !~ m/SPDX\-License\-Identifier/ } @lines;

    push(@code,@lines);
}

foreach my $header (@headers) {
    open(my $fh, '<', $header) or die $!;
    my @lines = map { trim_end($_) } <$fh>;
    close($fh);

    for(my $i=$#lines; $i >= 0; $i--) {
        if(length($lines[$i]) == 0) {
            delete($lines[$i]);
        } else {
            last;
        }
    }

    @lines = reverse @lines;

    for(my $i=$#lines; $i >= 0; $i--) {
        if(length($lines[$i]) == 0) {
            delete($lines[$i]);
        } else {
            last;
        }
    }

    shift @lines; # remove final #endif

    @lines = reverse @lines;

    shift @lines; # remove SPDX ID
    shift @lines; # remove #ifndef
    shift @lines; # remove #define

    @lines = grep { $_ !~ m/^\#include/ } @lines;

    push(@typedef_structs, grep { $_ =~ m/typedef struct/ } @lines);
    @lines = grep { $_ !~ m/typedef struct/ } @lines;

    push(@typedef_enums, grep { $_ =~ m/typedef enum/ } @lines);
    @lines = grep { $_ !~ m/typedef enum/ } @lines;

    my $state = 0;
    my $cpp_counter = 0;
    my $enum;
    my $function_declaration;
    my $struct;

    foreach my $line (@lines) {
        if($state == 0) {
            if ($line =~ m/^enum/) {
                $enum = [];
                $state = 1;
            } elsif($line =~ m/\#ifdef __cplusplus/) {
                $state = 2;
            } elsif($line =~ m/struct/) {
                $struct = [];
                $state = 5;
            }
        }

        if($state == 1) {
            push(@$enum,$line);
            if ($line =~ m/\};$/) {
                $state = 0;
                push(@enums,$enum);
            }
        }

        if($state == 2) {
            $cpp_counter++;
            if($cpp_counter == 3) {
                $state = 3;
                $function_declaration = [];
                next;
            }
        }

        if($state == 5) {
            push(@$struct,$line);
            if($line =~ m/^\};$/) {
                push(@structs,$struct);
                $struct = [];
                $state = 0;
            }
        }

        if($state == 3) {
            if($line =~ m/\#ifdef __cplusplus/) {
                $cpp_counter = 0;
                $state = 4;
            } else {
                if(length($line) > 0) {
                    push(@$function_declaration,$line);
                    if($line =~ m/;$/) {
                        push(@function_declarations,$function_declaration);
                        $function_declaration = [];
                    }
                }
            }
        }

        if($state == 4) {
            $cpp_counter++;
            if($cpp_counter == 3) {
                last;
            }
        }

    }

}

foreach my $function_declaration (@function_declarations) {
    my $kind = 0;
    foreach my $l (@$function_declaration) {
        if($l =~ m/MINIFLAC_API/) {
            $kind = 1;
            last;
        } elsif($l =~ m/MINIFLAC_PRIVATE/) {
            $kind = 2;
            last;
        }
    }
    if($kind == 0) {
        print Dumper $function_declaration;
        die "unknown function declaration";
    }
    if($kind == 1) {
        push(@public_function_declarations,$function_declaration);
    } else {
        push(@private_function_declarations,$function_declaration);
    }
}

my @unified;
open(my $fh, '<', 'src/miniflac.h') or die $!;
while(<$fh>) {
    my $line = trim_end($_);
    if($line =~ /^\#inject typedef_structs/) {
        push(@unified,@typedef_structs);
    } elsif($line =~ m/^\#inject typedef_enums/) {
        push(@unified,@typedef_enums);
    } elsif($line =~ m/^\#inject enums/) {
        foreach my $enum (@enums) {
            push(@unified,@$enum);
            push(@unified,'');
        }
    } elsif($line =~ m/^\#inject structs/) {
        foreach my $struct (@structs) {
            push(@unified,@$struct);
            push(@unified,'');
        }
    } elsif($line =~ m/^\#inject public_function_declarations/) {
        foreach my $decl (@public_function_declarations) {
            push(@unified,@$decl);
            push(@unified,'');
        }
    } elsif($line =~ m/^\#inject private_function_declarations/) {
        foreach my $decl (@private_function_declarations) {
            push(@unified,@$decl);
            push(@unified,'');
        }
    } elsif($line =~ m/^\#inject code/) {
        push(@unified,@code);
    } else {
        push(@unified,$line);
    }
}
close($fh);

print join("\n",@unified) . "\n";



