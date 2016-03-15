#!/bin/bash

if [[ -d ../scripts ]]; then cd ..; fi
mkdir -p res/maps res/ships res/snd res/music
cd res 
ftp -nv yoanlecoq.com << EOF
user hovercraft
passive
prompt
binary
newer ships/dolphin_unicorn.txt
newer ships/dolphin_unicorn.png
newer ships/dolphin_unicorn.pal.png
newer maps/bigblue.txt
newer maps/bigblue.png
bye
EOF
