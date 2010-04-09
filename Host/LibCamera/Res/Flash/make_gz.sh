#!/bin/bash

# make_gz.sh: Convert the flash to gz except some special folders, such
# as audio. 
# $Id$

APP_NAME="${0##*[\\/]}"
APP_VERSION="1.0"
APP_AUTHOR="ChenXiaohui"
APP_PATH="${0%[\\/]*}"

pushd "$APP_PATH"

outbase="../../../../Binary/FlashFiles"
outpath="Resource"
outini="FirmwareMaker.ini"

rm -rf "$outbase/$outpath"
mkdir -p "$outbase/$outpath"

tar -c --file - --exclude "$outbase/$outpath" --exclude "make_gz.sh" --exclude '.svn' --exclude "*.bak" --exclude "*~" "." | tar -x -v --file - --directory "$outbase/$outpath"

pushd "$outbase"
find "$outpath" -type f | sed -e '/\.[Ss][Vv][Nn]\//d' -e '/~$/d' -e '/\.[Bb][Aa][Kk]/d' | while read path; do
	if [ "${path:${#outpath}:7}" != "/audio/" ]; then
		gzip -9 "$path"
	fi
done

file_num="$(find "$outpath" -type f | wc -l)"

cat > "$outini" <<EOT
[PATH]
FOLDER=.
INPUT_BIN=..\img_spi.bin
OUTPUT_BIN=..\img_spi_update.bin
FILE_NUM=$file_num
EOT

i=0
find "$outpath" -type f | while read path; do
	echo "FILE_ONBOARD_$i=${path:$((${#outpath}+1))}"
	echo "FILE_LOCAL_$i=$(cygpath -w "${path}")"
#FILE_ONBOARD_1=audio/lowbattery.pcm
#FILE_LOCAL_1=flash_files\audio\lowbattery.pcm
	(( i++ ))
done >> "$outini"

popd

popd

