#!/bin/bash

PROJECTDIR=$1

echo ""

# stop existing docker processes
DOCKER_PROCESSES=$(docker ps -q)

if [ ! -z "${DOCKER_PROCESSES}" ]; then
    echo "stopping running docker processes..."
    echo ""
	/usr/bin/docker stop $DOCKER_PROCESSES
    echo ""
fi

# copy hooks, if exist
if [ -d "${PROJECTDIR}/hooks/git/" ]; then
    echo "copying git hooks..."
    cp -R ${PROJECTDIR}/hooks/git/* ${PROJECTDIR}/.git/hooks/
fi