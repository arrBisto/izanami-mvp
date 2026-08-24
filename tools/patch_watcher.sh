#!/data/data/com.termux/files/usr/bin/bash
LOCK=$HOME/.izanami_patch_watch.lock
if ! mkdir "$LOCK" 2>/dev/null; then echo "[watcher] already running"; exit 1; fi
trap 'rmdir "$LOCK"' EXIT
WATCH=~/storage/shared/Izanami/forge/patches
mkdir -p "$WATCH/done"
echo "[watcher] monitoring $WATCH"
while true; do
  for p in "$WATCH"/patch_*.sh; do
    [ -e "$p" ] || continue
    name=$(basename "$p")
    echo "[watcher] executing $name"
    bash "$p" > "$WATCH/$name.log" 2>&1
    echo "[watcher] exit=$? log=$name.log"
    mv "$p" "$WATCH/done/"
  done
  sleep 3
done
