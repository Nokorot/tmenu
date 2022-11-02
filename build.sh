#!/bin/bash

### Program name
PRGNAME="tmenu"

### Source files
SRC="$(find src -type f -name "*.c" )"
EXE="$PRGNAME"

### Compiler
CC="gcc"

### Compile definions
DEFS="-D_GNU_SOURCE"

### Compililation flags
STD="" # "-std=c99"
# LIBS="-ldwmapi -luxtheme"

DFLAGS="$STD $DEFS -Wall -pedantic -ggdb"
RFLAGS="$STD $DEFS -O3"

### Instalation destination
DST="/usr"



debug() {
  $CC $DFLAGS -o $EXE $SRC \
    || exit 1
  echo "Build debug complete!"
}

release() {
  $CC $RFLAGS -o $EXE $SRC
}

install() {
  cp $EXE $DST/bin/$EXE
  chmod 755 $DST/bin/$EXE
}

run() {
   ./$EXE "$@"
}

clean() {
  echo cleaning
  set -x
  rm -rf $EXE
}

usage() {
    echo "Usage: $0 [subcmd]\n"
    echo ""
    echo "Subcmds:"
    echo "    build|debug   building in debug mode"
    echo "    install       building in release and intaling the executable"
    echo "    clean         deleting build files"
}

set -e
if [ -z "$1" ]; then 
    debug
else
  case "$1" in
    build|debug)  debug ;;
    run)          debug && { shift; run "$@"; } ;;
    install)      release && install ;;
    clean)        clean ;;
  esac
fi


