#!/bin/bash

if [[ -d ../scripts ]]; then cd ..; fi
mkdir -p res/menus res/maps res/ships res/snd res/music
cd res 
ftp -nv yoanlecoq.com << EOF
user hovercraft
passive
prompt
binary
newer COPYRIGHTS.txt
newer menus/menus.txt
newer menus/menus.png
newer menus/main_menu_music.mp3
newer ships/dolphin_unicorn.txt
newer ships/dolphin_unicorn_pal.png
newer ships/dolphin_unicorn_tex.png
newer ships/dolphin_unicorn_artwork.png
newer ships/raging_waffle.txt
newer ships/raging_waffle_pal.png
newer ships/raging_waffle_tex.png
newer ships/raging_waffle_artwork.png
newer ships/regular_ship.txt
newer ships/regular_ship_pal.png
newer ships/regular_ship_tex.png
newer ships/regular_ship_artwork.png
newer ships/darkness_of_evil.txt
newer ships/darkness_of_evil_pal.png
newer ships/darkness_of_evil_tex.png
newer ships/darkness_of_evil_artwork.png
newer maps/bigblue.txt
newer maps/bigblue.png
newer maps/bigblue_tile.png
newer maps/Deflate_-_Phoenix.mp3
newer maps/zero.txt
newer maps/zero.png
newer maps/zero_tile.png
newer maps/Deflate_-_Something_Wrong.mp3
newer maps/slide_city.txt
newer maps/slide_city.png
newer maps/slide_city_tile.png
newer maps/Deflate_-_Direct_Access.mp3
newer maps/inferno.txt
newer maps/inferno.png
newer maps/inferno_tile.png
newer snd/menu_transition.wav
newer snd/clap_close.wav
bye
EOF
