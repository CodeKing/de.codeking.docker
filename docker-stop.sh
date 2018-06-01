#!/bin/bash

DOCKER_PROCESSES=$(docker ps -a -q)

if [ ! -z "${DOCKER_PROCESSES}" ]; then
	/usr/bin/docker stop $DOCKER_PROCESSES
fi
