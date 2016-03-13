#!/bin/bash

if [[ -d ../scripts ]]; then cd ..; fi
mkdir -p res 
cd res 
ftp -nv yoanlecoq.com << EOF
user hovercraft tout_le_monde_est_content
passive
prompt
binary
newer test.png
newer test2.png
bye
EOF
