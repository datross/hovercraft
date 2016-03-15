#!/bin/bash

if [[ -d ../res ]]; then cd ..; fi
cd res
ftp -nv yoanlecoq.com << EOF
user hovercraft
passive
prompt
binary
put ships/dolphin_unicorn.txt
put ships/dolphin_unicorn.png
put ships/dolphin_unicorn.pal.png
put maps/bigblue.txt
put maps/bigblue.png
bye
EOF
