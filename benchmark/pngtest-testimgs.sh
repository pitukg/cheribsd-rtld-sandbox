for sz in 1 2 4 8 16 32 64 128 256 512 1024 ; do
 LD_LIBRARY_PATH=~ ./ld-elf.so.1 ./pngtest --benchmark benchmark/pngtest/resized_testimgs/testpng$sz.png --libxo json 2>/dev/null > benchmark/results/pngtest-on-testpng$sz-baseline.json
 LD_LIBRARY_PATH=~ ./ld-elf.so.1 ./pngtest-sandbox --benchmark benchmark/pngtest/resized_testimgs/testpng$sz.png --libxo json 2>/dev/null > benchmark/results/pngtest-on-testpng$sz-sandbox.json
done
