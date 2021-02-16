#!/bin/bash

set -e

if [[ -z "$1" ]]; then
    echo "No filename supplied"
    exit
fi

REPORT_SERVER="pianosa"
FILE_PATH=$1
FILE_NAME=$(basename -- "$FILE_PATH")
FILE_NAME_NO_EXT="${FILE_NAME%.*}"
REPORT_PATH=report/"$FILE_NAME_NO_EXT"

echo "Remote compiling \"$FILE_NAME\""
echo "Coping to $REPORT_SERVER:~/$REPORT_PATH/$FILE_NAME"

REPORT_MKDIR_COMMAND="mkdir -p ~/$REPORT_PATH"
ssh "$REPORT_SERVER" "$REPORT_MKDIR_COMMAND"
scp "$FILE_PATH" "$REPORT_SERVER":~/"$REPORT_PATH/$FILE_NAME"

REPORT_COMPILE_COMMAND="cd ~/$REPORT_PATH;"
REPORT_COMPILE_COMMAND+="source /etc/profile.d/fpga.sh;"
REPORT_COMPILE_COMMAND+="time aoc -rtl $FILE_NAME -report;"
REPORT_COMPILE_COMMAND+="exit;"

ssh -o ServerAliveInterval=60 -t pianosa "$REPORT_COMPILE_COMMAND"

LOCAL_REPORT="/Volumes/RamDisk/$FILE_NAME_NO_EXT"
if [[ ! -z "$2" ]]; then
    LOCAL_REPORT="$2/$FILE_NAME_NO_EXT"
fi

mkdir -p "$LOCAL_REPORT"
rsync -rav "$REPORT_SERVER":~/"$REPORT_PATH/$FILE_NAME_NO_EXT/reports/*" "$LOCAL_REPORT/."
open "$LOCAL_REPORT/report.html"
