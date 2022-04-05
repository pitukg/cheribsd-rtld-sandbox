for ld in `ls ~/ablation`; do
  echo "Running $ld...";
  ~/ablation/$ld ~/benchmark/benchmark-function-call-sandbox --libxo json > ~/benchmark/results/ablation/$ld-sandbox.json;
  cp ~/benchmark/results/function-call-baseline.json ~/benchmark/results/ablation/$ld-baseline.json;
done
