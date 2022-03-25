for sz in 1 2 4 8 16 32 64 128 256 512 1024 ; do
 ./ld-elf.so.1 ~/bin/magick benchmark/pngtest/srcdir/contrib/testpngs/rgb-alpha-16.png -colorspace RGB +sigmoidal-contrast 11.6933 -define filter:filter=Sinc -define filter:window=Jinc -define filter:lobes=3 -resize $sz -sigmoidal-contrast 11.6933 -colorspace sRGB /tmp/testpng$sz.png
done
