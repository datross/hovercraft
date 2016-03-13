#!/bin/bash

if [[ -d ../res ]]; then cd ..; fi
cd res
ftp -nv yoanlecoq.com << EOF
user hovercraft tout_le_monde_est_content
passive
prompt
binary
mput *
bye
EOF
