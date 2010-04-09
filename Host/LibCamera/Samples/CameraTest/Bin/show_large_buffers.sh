#!/bin/bash

APP_PATH="${0%\/*}"

list_file="$APP_PATH/HIC_Test.lst"
if [ ! -f "$list_file" ]; then
	echo "Memory dump file $list_file not exists."
	exit 1;
fi

sed -n '/^[[:space:]]\+[_.A-Za-z\$][^[:space:]]*[[:space:]]\+0x[[:xdigit:]]\+[[:space:]]\+Data/p' "$list_file" | sort -n -k 4 | tail -n 20

