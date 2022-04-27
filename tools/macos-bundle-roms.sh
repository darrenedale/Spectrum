#!/bin/zsh

EXIT_OK=0
ERR_NO_BUNDLE_PATH=1

PROJECT_ROOT_DIR="$(dirname "${0}")/.."

if [ -z "${1}" ]; then
  echo >&2 "Path to app bundle is required."
  exit ${ERR_NO_BUNDLE_PATH}
fi

BUNDLE_PATH="${1}"

echo "Copying roms to '${BUNDLE_PATH}/Contents/roms/'"
cp -r "${PROJECT_ROOT_DIR}/roms" "${BUNDLE_PATH}/Contents/"
exit ${EXIT_OK}
