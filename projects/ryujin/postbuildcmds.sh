#!/bin/sh

input_dir=$1
output_dir=$2
build_cfg=$3

if [ -z "$3" ]; then
    build_cfg=""
fi

if [ ! -d $output_dir ]; then
    mkdir -p $output_dir
fi

for f in $(find $input_dir -name '*.vert' -or -name '*.vs' -or \
            -name '*.tcs' -or -name '*.tesc' -or -name '*.tes' -or \
            -name '*.tese' -or -name '*.geom' -or -name '*.gs' -or \
            -name '*.frag' -or -name '.fs' -or -name '*.comp' -or \
            -name '*.cs'); do
    if [ "$build_cfg" = "debug" ]; then
        glslc "$f" -o "$f.spv" -O0
    else
        if [ "$build_cfg" = "release" ]; then
            glslc "$f" -o "$f.spv" -O
        else
            glslc "$f" -o "$f.spv"
        fi
    fi
done

cp -r "$input_dir/." $output_dir

for f in $(find $output_dir -name '*.vert' -or -name '*.vs' -or \
            -name '*.tcs' -or -name '*.tesc' -or -name '*.tes' -or \
            -name '*.tese' -or -name '*.geom' -or -name '*.gs' -or \
            -name '*.frag' -or -name '.fs' -or -name '*.comp' -or \
            -name '*.cs'); do
    rm -rf "$f"
done