#!/bin/bash

set -e
# set -v

if [ -z "$1" ]; then
    echo "No filename supplied"
    exit
fi

COMPILE_REMOTE="pianosa"
FPGA_REMOTE="socfpga"
FILE_PATH=$1
FILE_NAME=$(basename -- "$FILE_PATH")
FILE_NAME_NO_EXT="${FILE_NAME%.*}"
COMPILE_REMOTE_PATH=compiled/"$FILE_NAME_NO_EXT"

echo "Remote compiling \"$FILE_NAME\""
echo "Coping to $COMPILE_REMOTE:$COMPILE_REMOTE_PATH/$FILE_NAME"

COMPILE_REMOTE_MKDIR_COMMAND="mkdir -p ~/compiled/$FILE_NAME_NO_EXT"
ssh "$COMPILE_REMOTE" "$COMPILE_REMOTE_MKDIR_COMMAND"
scp "$FILE_PATH" "$COMPILE_REMOTE":~/"$COMPILE_REMOTE_PATH/$FILE_NAME"

COMPILE_REMOTE_COMPILE_COMMAND="cd ~/$COMPILE_REMOTE_PATH;"
COMPILE_REMOTE_COMPILE_COMMAND+="source /etc/profile.d/fpga.sh;"
COMPILE_REMOTE_COMPILE_COMMAND+="time aoc -rtl $FILE_NAME -report;"
COMPILE_REMOTE_COMPILE_COMMAND+="exit;"

ssh -o ServerAliveInterval=60 -t pianosa "$COMPILE_REMOTE_COMPILE_COMMAND"

LOCAL_REPORT="/Volumes/RamDisk/compiled/$FILE_NAME_NO_EXT"
mkdir -p "$LOCAL_REPORT"
rsync -rav "$COMPILE_REMOTE":~/"$COMPILE_REMOTE_PATH/$FILE_NAME_NO_EXT/reports/*" "$LOCAL_REPORT/."
