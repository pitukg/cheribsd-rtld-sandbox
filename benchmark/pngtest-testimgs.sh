for sz in 1 2 4 8 16 32 64 128 256 512 1024 ; do
 echo "Running baseline on $sz"
 LD_LIBRARY_PATH=~ ./ld-elf.so.1 ./pngtest1000 --benchmark benchmark/pngtest/resized_testimgs/testpng$sz.png --libxo json 2>/dev/null > benchmark/results/pngtest-parameterised-1000/pngtest-on-testpng$sz-baseline.json
 echo "Running sandbox on $sz"
 LD_LIBRARY_PATH=~ ./ld-elf.so.1 ./pngtest-sandbox1000 --benchmark benchmark/pngtest/resized_testimgs/testpng$sz.png --libxo json 2>/dev/null > benchmark/results/pngtest-parameterised-1000/pngtest-on-testpng$sz-sandbox.json
done
