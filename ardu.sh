#!/bin/bash
docker run --rm -v "$(pwd):/ardupilot" -u "$(id -u):$(id -g)" ardupilot:latest bash -c "$@"
