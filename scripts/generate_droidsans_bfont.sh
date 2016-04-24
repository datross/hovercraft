#!/bin/bash

function genxbm {
    echo Generating ${1}...
    convert -font Droid-Sans-Mono -pointsize 24 label:$1 ${1}.xbm
    sed -i "s/${1}_/BitmapFont_DroidSansMono_${1}_/g" ${1}.xbm
    echo "#include \"${1}.xbm\"" >> ../BitmapFont_DroidSansMono.h
}

mkdir BitmapFont_DroidSansMono
cd BitmapFont_DroidSansMono
echo "#ifndef BITMAPFONT_DROIDSANSMONO_H"  > ../BitmapFont_DroidSansMono.h
echo "#define BITMAPFONT_DROIDSANSMONO_H" >> ../BitmapFont_DroidSansMono.h
for letter in {A..Z}; do genxbm $letter; done
for letter in {a..z}; do genxbm $letter; done
for letter in {0..9}; do genxbm $letter; done
sed -i "s/, \}/ \}/g" * 
sed -i "s/char/const GLubyte/g" * 
echo "#endif /* BITMAPFONT_DROIDSANSMONO_H */" >> ../BitmapFont_DroidSansMono.h
