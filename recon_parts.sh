#!/bin/bash
cd ~/Izanami || exit 1
part="$1"

print_missing() {
    echo "MISSING: $1"
}

case "$part" in

1)
    echo "=== PART 1: PROJECT STRUCTURE + MAIN.CPP HEAD ==="
    date

    echo
    echo "--- top-level project files ---"
    find . -maxdepth 1 -type f \( -name '*.sh' -o -name '*.md' -o -name 'CMakeLists.txt' \) 2>/dev/null | sort

    echo
    echo "--- native directories ---"
    find native -maxdepth 1 -type d 2>/dev/null | sort

    echo
    echo "--- native source files, excluding dependencies ---"
    find native -maxdepth 1 -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) 2>/dev/null | sort

    echo
    echo "--- CMake files, excluding dependencies ---"
    find . -maxdepth 3 -type f -name 'CMakeLists.txt' \
        ! -path './native/raylib/*' \
        ! -path './submodules/*' \
        ! -path './native/build/*' \
        2>/dev/null | sort

    echo
    echo "--- Java files ---"
    find native/java_src -maxdepth 3 -type f 2>/dev/null | sort

    echo
    echo "--- main.cpp total lines ---"
    if [ -f native/main.cpp ]; then
        wc -l native/main.cpp
        echo
        echo "--- native/main.cpp lines 1-220 ---"
        nl -ba native/main.cpp | sed -n '1,220p'
    else
        print_missing native/main.cpp
    fi
    ;;

2)
    echo "=== PART 2: MAIN.CPP TAIL ==="
    if [ -f native/main.cpp ]; then
        echo "--- native/main.cpp total lines ---"
        wc -l native/main.cpp
        echo
        echo "--- native/main.cpp lines 221-520 ---"
        nl -ba native/main.cpp | sed -n '221,520p'
    else
        print_missing native/main.cpp
    fi
    ;;

3)
    echo "=== PART 3: CHATENGINE BUILD_PROMPT REGION ==="
    f=native/ChatEngine.cpp

    if [ ! -f "$f" ]; then
        print_missing "$f"
        exit 0
    fi

    echo "--- $f total lines ---"
    wc -l "$f"

    echo
    echo "--- key anchors in $f ---"
    grep -nE "build_prompt|history|send|generate|router|image|vision|model|soul" "$f" | head -80

    line=$(grep -n "build_prompt" "$f" | head -1 | cut -d: -f1)

    if [ -n "$line" ]; then
        start=$((line > 35 ? line - 35 : 1))
        end=$((start + 260))

        echo
        echo "--- $f lines $start-$end around build_prompt match at line $line ---"
        nl -ba "$f" | sed -n "${start},${end}p"
    else
        echo
        echo "build_prompt not found. Showing first 220 lines instead."
        nl -ba "$f" | sed -n '1,220p'
    fi
    ;;

4)
    echo "=== PART 4: SOULCORE HEADER + SOULCORE.CPP HEAD ==="

    if [ -f native/SoulCore.hpp ]; then
        echo
        echo "--- native/SoulCore.hpp total lines ---"
        wc -l native/SoulCore.hpp
        echo
        echo "--- native/SoulCore.hpp lines 1-300 ---"
        nl -ba native/SoulCore.hpp | sed -n '1,300p'
    else
        print_missing native/SoulCore.hpp
    fi

    if [ -f native/SoulCore.cpp ]; then
        echo
        echo "--- native/SoulCore.cpp total lines ---"
        wc -l native/SoulCore.cpp
        echo
        echo "--- native/SoulCore.cpp lines 1-260 ---"
        nl -ba native/SoulCore.cpp | sed -n '1,260p'
    else
        print_missing native/SoulCore.cpp
    fi
    ;;

5)
    echo "=== PART 5: SOULCORE ANCHORS + UPDATE REGION ==="
    f=native/SoulCore.cpp

    if [ ! -f "$f" ]; then
        print_missing "$f"
        exit 0
    fi

    echo "--- key anchors in $f ---"
    grep -nE "Sarcasm detection|Silliness detection|Mischief|Intimacy Growth|base_exclam|boredom|locked_in|update|tone_of|state_to_json" "$f" | head -120

    line=$(grep -nE "::update|update\(" "$f" | head -1 | cut -d: -f1)

    if [ -n "$line" ]; then
        start=$((line > 30 ? line - 30 : 1))
        end=$((start + 320))

        echo
        echo "--- $f lines $start-$end around update match at line $line ---"
        nl -ba "$f" | sed -n "${start},${end}p"
    else
        echo
        echo "update function not found. Showing lines 261-580 instead."
        nl -ba "$f" | sed -n '261,580p'
    fi
    ;;

6)
    echo "=== PART 6: INFERENCE ENGINE PARAMS + GENERATE/HOT_SWAP ==="
    f=native/InferenceEngine.cpp

    if [ ! -f "$f" ]; then
        print_missing "$f"
        exit 0
    fi

    echo "--- $f total lines ---"
    wc -l "$f"

    echo
    echo "--- key anchors in $f ---"
    grep -nE "n_gpu_layers|n_ctx|n_threads|n_batch|n_ubatch|generate|hot_swap|mtmd|model_params|ctx_params" "$f" | head -120

    echo
    echo "--- $f lines 80-260 ---"
    nl -ba "$f" | sed -n '80,260p'
    ;;

7)
    echo "=== PART 7: MODEL ROUTER + SOUL.LUA + RECENT LOGS ==="

    if [ -f native/ModelRouter.cpp ]; then
        echo "--- native/ModelRouter.cpp total lines ---"
        wc -l native/ModelRouter.cpp
        echo
        echo "--- native/ModelRouter.cpp lines 1-260 ---"
        nl -ba native/ModelRouter.cpp | sed -n '1,260p'
    else
        print_missing native/ModelRouter.cpp
    fi

    echo
    echo "--- /sdcard/Izanami/soul.lua ---"
    if [ -f /sdcard/Izanami/soul.lua ]; then
        cat /sdcard/Izanami/soul.lua
    else
        print_missing /sdcard/Izanami/soul.lua
    fi

    echo
    echo "--- last 40 lines of /sdcard/Izanami/memory/soul_log.txt ---"
    if [ -f /sdcard/Izanami/memory/soul_log.txt ]; then
        tail -n 40 /sdcard/Izanami/memory/soul_log.txt
    else
        print_missing /sdcard/Izanami/memory/soul_log.txt
    fi
    ;;

*)
    echo "Usage: bash ~/Izanami/recon_parts.sh [1|2|3|4|5|6|7]"
    echo "Start with: bash ~/Izanami/recon_parts.sh 1"
    ;;

esac
