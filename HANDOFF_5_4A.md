# IZANAMI: MODEL ROUTER HANDOFF
Status: COMPLETE 2026-08-24.
/models picker (tags CHAT/TTS/IMG) over "Ai offline models" + Izanami/models. Async hot_swap_to_path (no ANR). Persistence: /sdcard/Izanami/memory/active_model.txt. Auto-Router: /autoroute on|off (autoroute.txt), keyword classifier CODE->Coder-7B, REASON->mistral-7B, CHAT->no swap. Trigger message answered by previous core; swap warms next.
Speed backlog: flash-attn, n_ctx trim, Q3 quants, thread pinning.
Next: 5.4-B Micro-Phase Executor; vision pending mmproj asset download.
