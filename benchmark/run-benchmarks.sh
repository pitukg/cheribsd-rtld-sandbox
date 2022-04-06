LD=~/ablation/ld-restrict-sp-fp-bounds.so.1

echo "Running dlopen benchmark..."
$LD ~/benchmark/benchmark-dlopen --libxo json > ~/benchmark/results/dlopen-dlclose-baseline.json
$LD ~/benchmark/benchmark-dlopen_sandbox --libxo json > ~/benchmark/results/dlopen-dlclose-sandbox.json

echo "Running dlsym benchmark..."
$LD ~/benchmark/benchmark-dlsym --libxo json > ~/benchmark/results/dlsym-baseline.json
$LD ~/benchmark/benchmark-dlsym-sandbox --libxo json > ~/benchmark/results/dlsym-sandbox.json

echo "Running function call benchmark..."
$LD ~/benchmark/benchmark-function-call --libxo json > ~/benchmark/results/function-call-baseline.json
$LD ~/benchmark/benchmark-function-call-sandbox --libxo json > ~/benchmark/results/function-call-sandbox.json

echo "Running lazy binding benchmark..."
$LD ~/benchmark/benchmark-lazy-binding --libxo json > ~/benchmark/results/lazy-binding-baseline.json
$LD ~/benchmark/benchmark-lazy-binding-sandbox --libxo json > ~/benchmark/results/lazy-binding-sandbox.json
