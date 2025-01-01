#!/bin/sh
#set -xe

ROOT_DIR="$(dirname $0)"
cd ${ROOT_DIR} # enter project dir

CC="gcc"

BUILD_DIR="./build"
STAMP_DIR="${BUILD_DIR}/.stamps"

list() {
    echo $@
}

SUB_FOLDERS=$(list "
    display
    ui
")

TEST_DIRS=$(list "
    tests
")

CFLAGS=$(list "
    -g
    -O0
    -std=gnu18
    -Wall
    -Wextra
    -Wno-override-init
    -Werror
")

CPPFLAGS=$(list "
    -I.
    -Ldeps
    -Ideps
")

SOURCES=$(list "
    tifc.c
    canvas.c
    input.c
    frame.c
")

LIBS=$(list "
    -lcircbuf_static
    -lhashmap_static
    -lsparse_static
    -ldynarr_static
    -lvector_static
    -lm
")

# Import subfolders
for dir in ${SUB_FOLDERS}; do
    #echo "Sub folder: ${dir}"

    imports=$(${dir}/import.sh)
    eval ${imports}

    # add subfolder to includes
    CPPFLAGS="${CPPFLAGS} -I${dir}"

    mkdir -p ${BUILD_DIR}/$(basename $dir)
    mkdir -p ${STAMP_DIR}/$(basename $dir)
    mkdir -p "/tmp/${STAMP_DIR}/$(basename $dir)"
done

for dir in ${TEST_DIRS}; do
    mkdir -p "${BUILD_DIR}/$(basename $dir)"
    mkdir -p ${STAMP_DIR}/$(basename $dir)
    mkdir -p "/tmp/${STAMP_DIR}/$(basename $dir)"
done

build_objects() {
    # Build objects from sources
    OBJECTS=""
    for src in ${1:-$SOURCES}; do
        # source -> object
        OBJ=$(echo ${src} | sed 's/\.c/\.o/g')

        # append to object list
        OBJECTS="${OBJECTS} ${BUILD_DIR}/${OBJ}"

        NEW_STAMP="/tmp/${STAMP_DIR}/${src}.sha1"
        OLD_STAMP="${STAMP_DIR}/${src}.sha1"

        sha1sum ${src} > ${NEW_STAMP} # recalculate stamp

        if [ -e "${OLD_STAMP}" ] && [ -z $(diff -q ${NEW_STAMP} ${OLD_STAMP}) ]; then
            : Skip ${OBJ}
        else
            # compile
            COMMAND="${CC} ${CPPFLAGS} ${CFLAGS} -c ${src} -o ${BUILD_DIR}/${OBJ}"
            ${COMMAND} && echo ${COMMAND}
            cp ${NEW_STAMP} ${OLD_STAMP} # refresh stamp
        fi
    done
    #echo "${OBJECTS}"
}

collect_tests() {
    # collect tests
    TESTS=""
    for test_dir in ${TEST_DIRS}; do
        for _test in ${test_dir}/*.c; do
            TESTS="${TESTS} ${_test}"
        done
    done
}

# Function takes source file path and returns list headers it depends on
get_dep_headers() {
    [ -z $1 ] && echo '!! get_dep_headers(): expects non-empty args' >&2 && exit 1
    wait
    HEADERS=$(${CC} ${CPPFLAGS} -MM $1 | sed "s/.*://; s/ [\\\\]//g;")
    echo "$HEADERS"
}

# Function takes source path and returns list sources it depends on
get_dep_sources() {
    [ -z $1 ] && echo '!! get_dep_sources(): expects non-empty args' >&2 && exit 1
    HEADERS=$(get_dep_headers $1)
    for header in ${HEADERS} ; do
        src=$(echo ${header} | sed 's/\.h/\.c/g')
        [ -e "./${src}" ] && echo "${src}"
    done
}

get_dep_objects() {
    [ -z $1 ] && echo '!! get_dep_objects(): expects non-empty args' >&2 && exit 1
    echo $(get_dep_sources $1) | sed 's/\.c/\.o/g'
}


help() {
    COMPILE_DESC="compile [<target>] - compile specific target. default target: 'tifc'"
    CLEAN_DESC="clean              - Remove all build artifacts."
    HELP_DESC="help               - show this help menu."
    case "$1" in
        compile) echo "${COMPILE_DESC}"
                 echo "Available targets:\n\ttifc\n\ttests"
        ;;
        clean)   echo "\t${CLEAN_DESC}"
        ;;
        help)    echo "\t${HELP_DESC}"
        ;;
        *)       echo "Available sub-commands:"
                 echo "\t${COMPILE_DESC}"
                 echo "\t${CLEAN_DESC}"
                 echo "\t${HELP_DESC}"
        ;;
    esac
}

SUB_CMD=${1:-'compile'}
case "${SUB_CMD}" in
    compile)
        TARGET=${2:-'tifc'}
        case "$TARGET" in
            tifc)
                build_objects
                COMMAND="${CC} ${CPPFLAGS} ${CFLAGS} ${OBJECTS} ${LIBS} -o ${BUILD_DIR}/tifc"
                echo "${COMMAND}"
            ;;
            tests)
                collect_tests
                echo "collected tests: $TESTS"
                for _test in ${TESTS}; do
                    SOURCES=$(get_dep_sources $_test)
                    echo "DEP SOURCES: ${SOURCES}"
                    TARGET=$(echo ${SOURCES} | cut -f1 -d' ' | sed 's/\.c//g')
                    build_objects "${SOURCES}"
                    COMMAND="${CC} ${CFLAGS} ${CPPFLAGS} ${OBJECTS} ${LIBS} -o ${BUILD_DIR}/${TARGET}"
                    echo "${COMMAND}" && $(${COMMAND})
                done
            ;;
            *) echo "ERROR : Wrong compile target '$2'" >&2
            ;;
        esac
    ;;
    clean)
        rm -rf ${BUILD_DIR}
        rm -rf ${STAMP_DIR}
    ;;
    help)
        help $2
    ;;
    *)
        echo "ERROR : Wrong sub-command '$SUB_CMD'" >&2
        help ${SUB_CMD} >&2
    ;;
esac

cd - > /dev/null # leave project dir
