#!/bin/bash
. ./base.sh
cd ..
make -f module.mk

# build a project
# move all the artifacts to build directory no matter what is the result of previous command
