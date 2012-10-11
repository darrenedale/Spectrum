#!/bin/bash
CURDIR=`pwd`
BASENAME=`basename ${CURDIR}`
ARCNAME=${BASENAME}_`date +%Y%m%d%H%M%S`.tar.gz
mkdir -p dist

echo "Building source distribution tarball \"${ARCNAME}\" in dist/"

tar --exclude="build" --exclude="dist" --exclude="build_tools" --exclude="doc" --exclude=".git" --exclude="*~" --exclude="*.bak" --exclude="*.autosave" --exclude="*.pro.user" --exclude="*.patch" --exclude="gruive/README" --exclude="Doxyfile" -czf "dist/${ARCNAME}" "../${BASENAME}"
