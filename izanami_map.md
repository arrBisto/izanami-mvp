# Izanami Self-Awareness Map
## Architecture
Izanami tools are authored in pure Lua. The core engine is written in C++ (Raylib + llama.cpp). 
The C++ core provides a secure execution hook that passes string parameters directly into the Lua stack.
## Tool Authoring Guidelines
1. Scripts must be placed in the `~/Izanami/scripts/` directory.
2. Scripts must accept an unparsed JSON parameter string as their first argument.
3. Scripts must return an absolute response string upon completion.
4. Upgrades and new packages can be pulled directly from the configured GitHub remote endpoint.
## Execution Flow
C++ Core -> LuaEngine::execute_script(script_path, json_params) -> Lua VM processes -> Returns string to C++
