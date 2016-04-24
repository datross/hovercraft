#!/bin/bash

if [[ -d ../scripts ]]; then cd ..; fi
mkdir -p res/menus res/maps res/ships res/snd res/music
cd res 
ftp -nv yoanlecoq.com << EOF
user hovercraft
passive
prompt
binary
newer menus/menus.txt
newer menus/menus.png
newer music/otherworld.flac
newer ships/dolphin_unicorn.txt
newer ships/dolphin_unicorn.png
newer ships/dolphin_unicorn.pal.png
newer ships/raging_waffle.txt
newer ships/raging_waffle.png
newer ships/raging_waffle.pal.png
newer ships/darkness_of_evil.txt
newer ships/darkness_of_evil.png
newer ships/darkness_of_evil.pal.png
newer ships/regular_ship.txt
newer ships/regular_ship.png
newer ships/regular_ship.pal.png
newer maps/bigblue.txt
newer maps/bigblue.png
newer maps/bigblue_tile.png
newer maps/dawn.txt
newer maps/dawn.png
newer maps/dawn_tile.png
bye
EOF
