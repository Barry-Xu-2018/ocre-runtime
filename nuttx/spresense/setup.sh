#!/bin/bash

# Registers Ocre as a Spresense SDK application root (SPRESENSE_HOME) and
# configures the SDK with SD card, Wi-Fi (GS2200M) and Ocre.
#
# Usage: setup.sh <spresense sdk repository> <approot directory> [extra configs...]
#
# Afterwards build with:
#   export SPRESENSE_HOME=<approot directory>
#   make -C <spresense sdk repository>/sdk

set -e

if [ $# -lt 2 ]; then
	echo "Usage: $0 <spresense sdk repository> <approot directory> [extra configs...]" >&2
	exit 1
fi

SPRESENSE_SDK=$(realpath "$1")
APPROOT=$(realpath -m "$2")
shift 2

OCRE_NUTTX=$(realpath "$(dirname "$0")/..")
SDK="${SPRESENSE_SDK}/sdk"

export SPRESENSE_HOME="${APPROOT}"

if [ ! -d "${APPROOT}" ]; then
	(cd "${SDK}" && ./tools/mkappsdir.py "${APPROOT}" "Ocre")
	touch "${APPROOT}/.approotdir"
fi

ln -sfn "${OCRE_NUTTX}" "${APPROOT}/ocre"

cd "${SDK}"

# 'preconfig' needs a configured kernel and adds SPRESENSE_HOME to sdk/Kconfig

if [ ! -f ../nuttx/.config ]; then
	./tools/config.py default
fi

make preconfig TOPDIR="${SPRESENSE_SDK}/nuttx" APPDIR="${SDK}/apps"

./tools/config.py default device/sdcard feature/wifi feature/ocre "$@"

grep -q '^CONFIG_OCRE=y' ../nuttx/.config || {
	echo "CONFIG_OCRE was not enabled" >&2
	exit 1
}

echo
echo "Configured. Build with:"
echo "  export SPRESENSE_HOME=${APPROOT}"
echo "  make -C ${SDK}"
