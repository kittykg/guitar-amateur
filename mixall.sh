#!/usr/bin/env bash

for file in "$@"
do
  ./mix.sh "$file"
done
