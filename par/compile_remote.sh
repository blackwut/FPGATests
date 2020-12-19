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
COMPILE_REMOTE_COMPILE_COMMAND+="time aoc -board=a10s_ddr -v -report -incremental -fast-compile -g -profile=all $FILE_NAME -o $FILE_NAME_NO_EXT.aocx;"

ssh -o ServerAliveInterval=60 -t pianosa "$COMPILE_REMOTE_COMPILE_COMMAND"
