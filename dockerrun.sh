#!/bin/bash

docker run --rm --security-opt seccomp=unconfined -v "$(pwd):/host" -w /host "$USER/598ape" "$@"
