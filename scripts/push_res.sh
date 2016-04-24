#!/bin/bash

if [[ -d ../scripts ]]; then cd ..; fi
mkdir -p res/menus res/maps res/ships res/snd res/music
cd res 
ftp -nv yoanlecoq.com << EOF
user hovercraft
passive
prompt
binary
put menus/menus.txt
put menus/menus.png
put ships/dolphin_unicorn.txt
put ships/dolphin_unicorn.png
put ships/dolphin_unicorn.pal.png
put ships/raging_waffle.txt
put ships/raging_waffle.png
put ships/raging_waffle.pal.png
put ships/darkness_of_evil.txt
put ships/darkness_of_evil.png
put ships/darkness_of_evil.pal.png
put ships/regular_ship.txt
put ships/regular_ship.png
put ships/regular_ship.pal.png
put maps/bigblue.txt
put maps/bigblue.png
put maps/bigblue_tile.png
put maps/dawn.txt
put maps/dawn.png
put maps/dawn_tile.png
bye
EOF
