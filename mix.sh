#!/usr/bin/env bash
DIR=$1
SEARCH_FILES=("rhythm.ogg" "guitar.ogg" "song.bak.ogg")
FOUND_FILES=()
NUM_FILES=0

# backup the original song.ogg
mv --no-clobber "$DIR/song.ogg" "$DIR/song.bak.ogg"

for FILE in "${SEARCH_FILES[@]}"
do
  if [ -f "$DIR/$FILE" ]; then
    FOUND_FILES+=("-i")
    FOUND_FILES+=("$DIR/$FILE")
    let "NUM_FILES+=1"
  fi
done

ffmpeg "${FOUND_FILES[@]}" -filter_complex amix=inputs="$NUM_FILES" -ac 2 \
  "$DIR/song.ogg"
