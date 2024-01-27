#!/bin/bash
#
# Wrapper script for emscripten build.
#

make_wrapper_script() {
    local toolname emtoolname path
    toolname="$1"
    emtoolname="$2"
    path="$EM_CACHE/bin/wasm32-unknown-emscripten-$toolname"
    (echo "#!/bin/bash"
     echo "exec $emtoolname \"\$@\"") > "$path"
    chmod a+rx "$path"
}

DEFAULT_CACHE_DIR=$(em-config CACHE)

export EM_CACHE=$PWD/emscripten_cache
export EM_FROZEN_CACHE=
export PKG_CONFIG_PATH=$EM_CACHE/pkg
export PATH="$EM_CACHE/bin:$PATH"
export CFLAGS="-sASYNCIFY -sENVIRONMENT=web"
export LDFLAGS="-flto $CFLAGS"

# The first time creating the cache directory, we make a copy of the
# one already installed on the system, to avoid having to rebuild all
# the system libraries.
if [ ! -e $EM_CACHE ]; then
    cp -R "$DEFAULT_CACHE_DIR" "$EM_CACHE"
fi

mkdir -p $EM_CACHE/pkg $EM_CACHE/bin

echo "
prefix=/
exec_prefix=/
libdir=/
includedir=/

Name: sdl2
Description: sdl2
Version: 2.20.0
Requires:
Conflicts:
Libs: -sUSE_SDL=2
Cflags: -sUSE_SDL=2
" > $PKG_CONFIG_PATH/sdl2.pc

make_wrapper_script ar     emar
make_wrapper_script gcc    emcc
make_wrapper_script g++    em++
make_wrapper_script ld     emcc
make_wrapper_script nm     emnm
make_wrapper_script ranlib emranlib

./autogen.sh --host=wasm32-unknown-emscripten
make
