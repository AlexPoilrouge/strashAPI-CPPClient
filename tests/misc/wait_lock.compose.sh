#!/bin/bash

LOCK_FILE_ABS_PATH="/app/locker/wait"

NB_TRY=0

while [ ! -f "${LOCK_FILE_ABS_PATH}" ]; do
    echo "Waiting for test-api initialization…"
    sleep 3
    if [ "${NB_TRY}" -ge 13 ]; then
        break
    fi
    NB_TRY="$(( NB_TRY + 1 ))"
done
echo -e "\tGo!"

BUILD_TYPE="$1"
shift;

$@

if [ "${BUILD_TYPE,,}" != "debug" ]; then
    curl "http://testapi:8080/stop"
fi
