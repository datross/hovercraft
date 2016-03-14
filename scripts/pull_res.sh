#!/bin/bash

if [[ -d ../scripts ]]; then cd ..; fi
mkdir -p res 
cd res 
ftp -nv yoanlecoq.com << EOF
user hovercraft
passive
prompt
binary
newer test.png
newer test2.png
bye
EOF
