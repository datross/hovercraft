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
newer menus/main_menu_music.mp3
newer ships/dolphin_unicorn.txt
newer ships/dolphin_unicorn_pal.png
newer ships/dolphin_unicorn_tex.png
newer ships/dolphin_unicorn_artwork.png
newer ships/raging_waffle.txt
newer ships/darkness_of_evil.txt
newer ships/regular_ship.txt
newer ships/1x1.png
newer maps/bigblue.txt
newer maps/bigblue.png
newer maps/bigblue_tile.png
newer maps/dawn.txt
newer maps/dawn.png
newer maps/dawn_tile.png
newer maps/pensif_race.mp3
newer snd/menu_transition.wav
newer snd/clap_close.wav
bye
EOF
