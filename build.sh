#!/bin/sh
#set -xe

ROOT_DIR="$(dirname $0)"
cd ${ROOT_DIR} # enter project dir

CC="gcc"

BUILD_DIR="./build"
STAMP_DIR="${BUILD_DIR}/.stamps"

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
    mkdir -p ${STAMP_DIR}/$(basename $dir)
    mkdir -p "/tmp/${STAMP_DIR}/$(basename $dir)"
done

build_objects() {
    # Build objects from sources
    OBJECTS=""
    for src in ${SOURCES}; do
        # source -> object
        OBJ=$(echo ${src} | sed 's/\.c/\.o/g')

        # append to object list
        OBJECTS="${OBJECTS} ${BUILD_DIR}/${OBJ}"

        NEW_STAMP="/tmp/${STAMP_DIR}/${src}.sha1"
        OLD_STAMP="${STAMP_DIR}/${src}.sha1"

        sha1sum ${src} > ${NEW_STAMP} # recalculate stamp

        if [ -e "${OLD_STAMP}" ] && [ -z $(diff -q ${NEW_STAMP} ${OLD_STAMP}) ]; then
            echo Skip ${OBJ}
        else
            # compile
            COMMAND="${CC} ${CPPFLAGS} ${CFLAGS} -c ${src} -o ${BUILD_DIR}/${OBJ}"
            ${COMMAND} && echo ${COMMAND}
            cp ${NEW_STAMP} ${OLD_STAMP} # refresh stamp
        fi
    done
    #echo "OBJECTS: ${OBJECTS}"
}

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
        build_objects
        gcc ${CPPFLAGS} ${CFLAGS} ${OBJECTS} ${LIBS} -o ${BUILD_DIR}/tifc
    ;;
    "clean")
        rm -rf ${BUILD_DIR}
        rm -rf ${STAMP_DIR}
    ;;
esac

cd - #leave project dir
