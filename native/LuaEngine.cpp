#include "LuaEngine.hpp"
#include <iostream>

LuaEngine::~LuaEngine() {
    if (lua_state_) lua_close(lua_state_);
}

void LuaEngine::initialize() {
    if (lua_state_) return;
    lua_state_ = luaL_newstate();
    if (lua_state_) luaL_openlibs(lua_state_);
}

std::string LuaEngine::execute_script(const std::string& script_path, const std::string& json_params) {
    if (!lua_state_) return "Error: Lua VM not initialized.";
    if (luaL_dofile(lua_state_, script_path.c_str()) != LUA_OK) {
        std::string err = lua_tostring(lua_state_, -1);
        lua_pop(lua_state_, 1);
        return "Lua Load Error: " + err;
    }
    lua_getglobal(lua_state_, "main");
    if (!lua_isfunction(lua_state_, -1)) return "Error: 'main' function not found in script.";
    lua_pushstring(lua_state_, json_params.c_str());
    if (lua_pcall(lua_state_, 1, 1, 0) != LUA_OK) {
        std::string err = lua_tostring(lua_state_, -1);
        lua_pop(lua_state_, 1);
        return "Lua Runtime Error: " + err;
    }
    std::string result = "";
    if (lua_isstring(lua_state_, -1)) result = lua_tostring(lua_state_, -1);
    lua_pop(lua_state_, 1);
    return result;
}
