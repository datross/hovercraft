#!/bin/bash

if [[ -d ../res ]]; then cd ..; fi
cd res
ftp -nv yoanlecoq.com << EOF
user hovercraft
passive
prompt
binary
mput *
bye
EOF
