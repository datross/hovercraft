#!/bin/bash

if [[ -d ../scripts ]]; then cd ..; fi
mkdir -p res/menus res/maps res/ships res/snd
cd res 
ftp -nv yoanlecoq.com << EOF
user hovercraft
passive
prompt
binary
put ships/dolphin_unicorn.txt
put ships/dolphin_unicorn_pal.png
put ships/dolphin_unicorn_tex.png
put ships/dolphin_unicorn_artwork.png
put ships/raging_waffle.txt
put ships/raging_waffle_pal.png
put ships/raging_waffle_tex.png
put ships/raging_waffle_artwork.png
put ships/regular_ship.txt
put ships/regular_ship_pal.png
put ships/regular_ship_tex.png
put ships/regular_ship_artwork.png
put ships/darkness_of_evil.txt
put ships/darkness_of_evil_pal.png
put ships/darkness_of_evil_tex.png
put ships/darkness_of_evil_artwork.png
put maps/bigblue.txt
put maps/bigblue.png
put maps/bigblue_tile.png
put maps/zero.txt
put maps/zero.png
put maps/zero_tile.png
put maps/slide_city.txt
put maps/slide_city.png
put maps/slide_city_tile.png
put maps/inferno.txt
put maps/inferno.png
put maps/inferno_tile.png
put snd/menu_transition.wav
bye
EOF
