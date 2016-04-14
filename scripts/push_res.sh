#!/bin/bash

if [[ -d ../res ]]; then cd ..; fi
cd res
ftp -nv yoanlecoq.com << EOF
user hovercraft
passive
prompt
binary
put music/otherworld.flac
put ships/dolphin_unicorn.txt
put ships/dolphin_unicorn.png
put ships/dolphin_unicorn.pal.png
put ships/raging_waffle.txt
put ships/raging_waffle.png
put ships/raging_waffle.pal.png
put ships/darkness_of_evil.txt
put ships/darkness_of_evil.png
put ships/darkness_of_evil.pal.png
put ships/completely_regular_ship.txt
put ships/completely_regular_ship.png
put ships/completely_regular_ship.pal.png
put maps/bigblue.txt
put maps/bigblue.png
put maps/dawn.txt
put maps/dawn.png
bye
EOF
