#!/bin/bash

### Program name
PRGNAME="tmenu"

### Source files
SRC="$(find src -type f -name "*.c" )"
EXE="$PRGNAME"

### Compiler
CC="gcc"

### Compile definions
DEFS="-ljansson -D_GNU_SOURCE"

### Compililation flags
STD="" # "-std=c99"
# LIBS="-ldwmapi -luxtheme"

DFLAGS="$STD $DEFS -Wall -pedantic -ggdb"
RFLAGS="$STD $DEFS -O3"

### Instalation destination
DST="/usr"


debug() {
  [ -a "$DEBUG_LOG" ] && DFLAGS="$DFLAGS -DDEBUG_LOG=\"$DEBUG_LOG\""
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

# Read arguments
POSITIONAL=()
while [[ $# -gt 0 ]]; do
case $1 in
    --debug-log) DEBUG_LOG="$2"; shift ;;
    -h|--help) _usage; exit 0 ;;
    --) shift; break ;;
    -*) echo "Invalid option '$1'" > /dev/stderr; exit 1 ;;
    *) POSITIONAL+=("$1") ;;
esac; shift;
done
set -- "$@${POSITIONAL[@]}"

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


