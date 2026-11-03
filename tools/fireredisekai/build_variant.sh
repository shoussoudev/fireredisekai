#!/bin/sh
# Build a developer variant that starts a new game at a checkpoint, then restore the normal build.
# usage: tools/fireredisekai/build_variant.sh <START_AT number> <output.gba>
set -e
cfg=include/config/fireredisekai.h
sed -i "s/^#define ISEKAI_START_AT .*/#define ISEKAI_START_AT $1/" $cfg
make firered -j2 > /tmp/variant_build.log 2>&1 || { sed -i "s/^#define ISEKAI_START_AT .*/#define ISEKAI_START_AT ISEKAI_START_NORMAL/" $cfg; tail -20 /tmp/variant_build.log; exit 1; }
cp pokefirered.gba "$2"
sed -i "s/^#define ISEKAI_START_AT .*/#define ISEKAI_START_AT ISEKAI_START_NORMAL/" $cfg
make firered -j2 > /tmp/normal_build.log 2>&1
echo "variant $1 -> $2"
