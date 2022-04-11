NUM_PNGS=`cat ~/valid-cam-testpngs.txt | wc -l | xargs`
cnt=0

mkdir -p /tmp/benchmark-magick

for png in `cat ~/valid-cam-testpngs.txt`; do
  cnt=$((cnt+1))
  printf "$cnt/$NUM_PNGS\r" >&2

  echo $png

  # First workload: read & write
  # BASELINE:
  LD_LIBRARY_PATH=~/benchmark/magick-baseline/lib:~/lib time-precise ~/ld-elf.so.1 ~/aarch64-unknown-freebsd13-magick \
        ~/cam-testpngs/$png /tmp/benchmark-magick/$png-1-baseline
  # SANDBOX:
  LD_LIBRARY_PATH=~/lib time-precise ~/ld-elf.so.1 ~/aarch64-unknown-freebsd13-magick \
        ~/cam-testpngs/$png /tmp/benchmark-magick/$png-1-sandbox

  # Second workload: read & resize & write
  # BASELINE:
  LD_LIBRARY_PATH=~/benchmark/magick-baseline/lib:~/lib time-precise ~/ld-elf.so.1 ~/aarch64-unknown-freebsd13-magick \
        ~/cam-testpngs/$png -resize 512 /tmp/benchmark-magick/$png-2-baseline
  # SANDBOX:
  LD_LIBRARY_PATH=~/lib time-precise ~/ld-elf.so.1 ~/aarch64-unknown-freebsd13-magick \
        ~/cam-testpngs/$png -resize 512 /tmp/benchmark-magick/$png-2-sandbox

  # Third workload: read & resize improved & write
  # BASELINE:
  LD_LIBRARY_PATH=~/benchmark/magick-baseline/lib:~/lib time-precise ~/ld-elf.so.1 ~/aarch64-unknown-freebsd13-magick \
        ~/cam-testpngs/$png -colorspace RGB +sigmoidal-contrast 11.6933 \
        -define filter:filter=Sinc -define filter:window=Jinc -define filter:lobes=3 \
        -resize 512 -sigmoidal-contrast 11.6933 -colorspace sRGB /tmp/benchmark-magick/$png-3-baseline
  # SANDBOX:
  LD_LIBRARY_PATH=~/lib time-precise ~/ld-elf.so.1 ~/aarch64-unknown-freebsd13-magick \
        ~/cam-testpngs/$png -colorspace RGB +sigmoidal-contrast 11.6933 \
        -define filter:filter=Sinc -define filter:window=Jinc -define filter:lobes=3 \
        -resize 512 -sigmoidal-contrast 11.6933 -colorspace sRGB /tmp/benchmark-magick/$png-3-sandbox

done
