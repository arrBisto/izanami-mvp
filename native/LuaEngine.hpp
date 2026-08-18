#pragma once
#include <string>
#include <lua.hpp>

class LuaEngine {
public:
    static LuaEngine& get_instance() {
        static LuaEngine instance;
        return instance;
    }
    void initialize();
    std::string execute_script(const std::string& script_path, const std::string& json_params);
private:
    LuaEngine() = default;
    ~LuaEngine();
    LuaEngine(const LuaEngine&) = delete;
    LuaEngine& operator=(const LuaEngine&) = delete;
    lua_State* lua_state_{nullptr};
};
