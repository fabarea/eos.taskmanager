#!/bin/bash

inotifywait -q -m -e close_write *.cpp |
while read -r filename event; do
  echo ""
  echo "File changed ${filename}. Compiling..."
  echo ""
  make
  echo ""
  echo "Done! Your turn..."
done
