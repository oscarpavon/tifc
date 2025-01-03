#!/bin/sh
#set -xe

ROOT_DIR="$(dirname $0)"
CC="gcc"

BUILD_DIR="./build"
STAMP_DIR="${BUILD_DIR}/.stamps"

list() { echo $@ ;}

SUBDIRS=$(list "
    display
    ui
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

init_subdirs() {
    for dir in ${SUBDIRS}; do
        CPPFLAGS="${CPPFLAGS} -I${dir}"
        mkdir -p ${BUILD_DIR}/$(basename $dir)
        mkdir -p ${STAMP_DIR}/$(basename $dir)
        mkdir -p "/tmp/${STAMP_DIR}/$(basename $dir)"
    done
}

build_objects() {
    # Build objects from sources
    local objects=""
    for src in $@; do
        # source -> object
        local obj=$(echo ${src} | sed 's/\.c/\.o/g')
        local deps=$( collect_dependencies ${src} )

        # append to object list
        objects="${objects} ${BUILD_DIR}/${obj}"

        local stamp="${STAMP_DIR}/${src}.sha1"

        if [ -e "${stamp}" ] \
        && [ 0 = $(sha1sum -c --status ${stamp}; echo $?) ]; then
            : echo "skipping ${obj}" >&2
        else
            # compile
            local cmd="${CC} ${CPPFLAGS} ${CFLAGS} -c ${src} -o ${BUILD_DIR}/${obj}"
            ${cmd} || return $? # return on failure
            echo "${cmd}" >&2
            sha1sum ${deps} > ${stamp} # recalculate stamp
        fi
    done
    echo "${objects}"
}

collect_tests() {
    # collect tests
    local tests=""
    for dir in ${SUBDIRS}; do
        for _test_ in ${dir}/*_test.c; do
            tests="${tests} ${_test_}"
        done
    done
    echo "$tests"
}

collect_dependencies() {
    [ $# = 0 ] && { echo "! collect_dependencies() expects source. " >&2 && exit 1 ;}

    # get list of header separated with space
    local dependencies=$( ${CC} ${CPPFLAGS} -MM $1 | tr -d '\n' \
        | sed 's/ [\]//g;' \
        | sed 's/.*: //g' \
        | xargs -n1 | sort -u | xargs )

    echo ${dependencies}
}

h2c() {
    local dep_sources=""
    for header in "$@"; do
        local src=$(echo ${header} | sed 's/\.h/\.c/')
        [ -e ${src} -a ${source} != ${src} ] && dep_sources="${dep_sources} ${src}"
    done
    echo ${dep_sources}
}

# Function takes executable source file path
# and returns a compile command
build_executable() {
    [ $# = 0 ] && { echo "! build_executable() expects source. " >&2 && exit 1 ;}
    local source="$1"

    local target=$( echo "${source}" | sed 's/\.c//')
    echo "Target: $target" >&2

    local deps=$( collect_dependencies ${source} )
    local sources="${source} $( h2c ${deps} )"
    local stamp="${STAMP_DIR}/${target}.sha1"

    local objects=$( build_objects ${sources} )
    [ $? != 0 ] && return $?

    if [ -e "${stamp}" ] \
    && [ 0 = $( sha1sum -c --status "${stamp}"; echo $? ) ]; then
        echo "Nothing to be done for target '${target}'" >&2
    else
        # compile
        local cmd="${CC} ${CPPFLAGS} ${CFLAGS} ${objects} -o ${BUILD_DIR}/${target} ${LIBS}"
        ${cmd} || return $? # return on failure
        echo ${cmd} >&2

        local obj_stamps=$( for src in ${sources}; do echo "${STAMP_DIR}/${src}.sha1"; done )
        sha1sum ${obj_stamps} > ${stamp}
    fi

    return 0
}

help() {
    local compile_desc="compile [<target>] - compile specific target. default target: 'tifc'"
    local clean_desc="clean              - Remove all build artifacts."
    local help_desc="help               - show this help menu."
    case "$1" in
        compile)
            echo "${compile_desc}"
            echo "Available targets:\n\ttifc\n\ttests"
        ;;
        clean)
            echo "\t${clean_desc}"
        ;;
        help)
            echo "\t${help_desc}"
        ;;
        *)
            echo "Available sub-commands:"
            echo "\t${compile_desc}"
            echo "\t${clean_desc}"
            echo "\t${help_desc}"
        ;;
    esac
}

main() {
    local sub_cmd=${1:-'compile'}
    case "${sub_cmd}" in
        compile)
            local target=${2:-'tifc'}
            case "$target" in
                tifc)
                    { build_executable 'tifc.c' ;}
                    [ $? != 0 ] && exit $?
                ;;
                tests)
                    local tests=$( collect_tests )
                    for _test_ in ${tests}; do
                        { build_executable ${_test_} ;}
                    done
                ;;
                *) echo "ERROR : Wrong compile target '$2'" >&2
                ;;
            esac
        ;;
        clean)
            local cmd="rm -rf ${BUILD_DIR}"
            echo "${cmd}"
            $($cmd)
        ;;
        help)
            { help $2 ;}
        ;;
        *)
            echo "ERROR : Wrong sub-command '${sub_cmd}'" >&2
            { help ${sub_cmd} >&2 ;}
        ;;
    esac
}

{
    cd ${ROOT_DIR}      # enter project dir
    init_subdirs
    main $@
    cd - > /dev/null    # leave project dir
}

