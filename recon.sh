#!/bin/bash
OUT=~/Izanami/recon_output.txt
rm -f $OUT
echo "=== IZANAMI RECON START ===" > $OUT
echo "Date: $(date)" >> $OUT

echo -e "\n=== 1. CORE NATIVE FILES ===" >> $OUT
for f in main.cpp ChatEngine.cpp ChatEngine.hpp SoulCore.cpp SoulCore.hpp InferenceEngine.cpp InferenceEngine.hpp ModelRouter.cpp; do
    if [ -f ~/Izanami/native/$f ]; then
        echo -e "\n--- FILE: native/$f ---" >> $OUT
        cat ~/Izanami/native/$f >> $OUT
    else
        echo -e "\n--- FILE: native/$f NOT FOUND ---" >> $OUT
    fi
done

echo -e "\n=== 2. SOUL.LUA ===" >> $OUT
if [ -f /sdcard/Izanami/soul.lua ]; then
    cat /sdcard/Izanami/soul.lua >> $OUT
else
    echo "soul.lua not found at /sdcard/Izanami/" >> $OUT
fi

echo -e "\n=== 3. RECENT SOUL LOGS ===" >> $OUT
if [ -f /sdcard/Izanami/memory/soul_log.txt ]; then
    tail -n 50 /sdcard/Izanami/memory/soul_log.txt >> $OUT
else
    echo "soul_log.txt not found." >> $OUT
fi

echo -e "\n=== IZANAMI RECON END ===" >> $OUT
echo "Recon complete. Saved to $OUT"
