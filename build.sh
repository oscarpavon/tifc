#!/bin/sh
set -xe

CC="gcc"

BUILD_DIR="./build"

SUB_FOLDERS="
    display
    ui
"

CFLAGS="
    -g
    -O0
    -std=gnu18
    -Wall
    -Wextra
    -Wno-override-init
    -Werror
"

CPPFLAGS="
    -Ldeps
    -Ideps
    -I.
"

SOURCES="
    tifc.c
    canvas.c
    input.c
    frame.c
"

# Import subfolders
for dir in ${SUB_FOLDERS}; do
    echo "Sub folder: ${dir}"

    imports=$(${dir}/import.sh)
    eval ${imports}

    # add subfolder to includes
    CPPFLAGS="${CPPFLAGS} -I${dir}"

    mkdir -p ${BUILD_DIR}/$(basename $dir)
done

# Build objects from sources
OBJECTS=""
for src in ${SOURCES}; do
    # source -> object
    OBJ=$(echo ${src} | sed 's/\.c/\.o/g')

    # append to object list
    OBJECTS="${OBJECTS} ${BUILD_DIR}/${OBJ}"

    # compile
    ${CC} ${CPPFLAGS} ${CFLAGS} -c ${src} -o ${BUILD_DIR}/${OBJ}
done
echo "OBJECTS: ${OBJECTS}"

LIBS="
    -lcircbuf_static
    -lhashmap_static
    -lsparse_static
    -ldynarr_static
    -lvector_static
    -lm
"

OPTION=${1:-'compile'}
case ${OPTION} in
    "compile")
        gcc ${CPPFLAGS} ${CFLAGS} ${OBJECTS} ${LIBS} -o ${BUILD_DIR}/tifc
    ;;
    "clean")
        rm -rf ${BUILD_DIR}
    ;;
esac
