#!/bin/sh

DIR=$(dirname $0)
MODULE=$(basename ${DIR} | tr '[:lower:]' '[:upper:]')
EXPORT=${MODULE}_SOURCES
SOURCES="$(for f in ${DIR}/*.c ; do echo $f; done)"

# export <MODULE>_SOURCES variable
printf "%s='%s'\n" ${EXPORT} ${SOURCES}

# append sources to SOURCES variable
printf "SOURCES=\"\$SOURCES %s\"\n" ${SOURCES}

