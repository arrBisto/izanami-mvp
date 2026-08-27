#include "SoulCore.hpp"
#include "LuaEngine.hpp"
#include <sqlite3.h>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <set>
#include <algorithm>
#include <cctype>

static const char* DB = "/sdcard/Izanami/memory/vault.db";

std::string SoulCore::escape_json(const std::string& s) {
    std::string o;
    for (char c : s) {
        if (c == '"') o += "'";
        else if (c == '\n') o += " ";
        else if (c == '\\') o += "/";
        else o += c;
    }
    return o;
}

void SoulCore::ensure_table() {
    sqlite3* db = nullptr;
    if (sqlite3_open(DB, &db) != SQLITE_OK) { if (db) sqlite3_close(db); return; }
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS episodes(id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp TEXT DEFAULT (datetime('now')), prompt TEXT, response TEXT, emotions TEXT, salience REAL DEFAULT 0.5, tags TEXT);", nullptr, nullptr, nullptr);
    sqlite3_close(db);
}

float SoulCore::salience_of(const std::string& t) const {
    float s = 0.3f;
    s += std::min((float)t.size() / 300.0f, 0.2f);
    if (t.find('?') != std::string::npos) s += 0.1f;
    if (t.find('!') != std::string::npos) s += 0.15f;
    int upper = 0, letters = 0;
    for (char c : t) { if (isalpha((unsigned char)c)) { letters++; if (isupper((unsigned char)c)) upper++; } }
    if (letters > 4) s += 0.2f * ((float)upper / letters);
    std::string low = t; std::transform(low.begin(), low.end(), low.begin(), ::tolower);
    const char* emo[] = {"love","hate","angry","happy","sad","mad","wow","amazing","terrible","miss","funny","scared"};
    for (auto w : emo) if (low.find(w) != std::string::npos) { s += 0.15f; break; }
    return std::min(s, 1.0f);
}

static std::set<std::string> tokenize(const std::string& s) {
    static const std::set<std::string> stop = {"about","today","hello","what","how","you","the","and","that","this","with","your","have","has","was","were","are","is","it","for","not","but","all","can","will","just","really","very","there","here","when","where","who","why","yeah","yes","nope","remember","tell","me"};
    std::set<std::string> out;
    std::string cur;
    for (char c : s) {
        if (isalnum((unsigned char)c)) cur += (char)tolower((unsigned char)c);
        else { if (cur.size() >= 4 && !stop.count(cur)) out.insert(cur); cur.clear(); }
    }
    if (cur.size() >= 4 && !stop.count(cur)) out.insert(cur);
    return out;
}

static int count_emoji(const std::string& s) {
    int n = 0;
    for (size_t i = 0; i + 1 < s.size(); i++) {
        if ((unsigned char)s[i] == 0xF0 && (unsigned char)s[i+1] == 0x9F) { n++; i += 3; }
    }
    if (s.find("\xE2\x9D\xA4") != std::string::npos) n++;
    if (s.find(":)") != std::string::npos) n++;
    if (s.find(":D") != std::string::npos) n++;
    if (s.find("XD") != std::string::npos) n++;
    return n;
}

SoulCore::Tone SoulCore::tone_of(const std::string& msg) {
    Tone t;
    float len = (float)msg.size();
    t.confidence = std::min(len / 80.0f, 1.0f);
    if (t.confidence < 0.15f) t.confidence = 0.15f;

    int letters = 0, upper = 0, exclam = 0;
    for (char c : msg) {
        if (isalpha((unsigned char)c)) { letters++; if (isupper((unsigned char)c)) upper++; }
        else if (c == '!') exclam++;
    }
    float caps = letters > 3 ? (float)upper / letters : 0.0f;
    float d_caps = caps - base_caps_;
    float d_ex = (float)exclam - base_exclam_;
    float d_em = (float)count_emoji(msg);
    base_caps_ = 0.9f * base_caps_ + 0.1f * caps;
    base_exclam_ = 0.9f * base_exclam_ + 0.1f * (float)exclam;

    float energy = d_caps * 2.0f + d_ex * 0.3f + d_em * 0.3f + caps * 0.5f;
    t.arousal = std::min(std::max(energy, 0.0f), 1.0f);

    std::string low = " " + msg + " ";
    std::transform(low.begin(), low.end(), low.begin(), ::tolower);
    auto hits = [&](std::initializer_list<const char*> ws) {
        int n = 0;
        for (auto w : ws) if (low.find(w) != std::string::npos) n++;
        return n;
    };
    int joy = hits({"happy","awesome","amazing","lol","haha","yay","great","cool","wow"});
    int anger = hits({"angry","mad","furious","hate","annoyed","pissed"});
    int sad = hits({"sad","tired","exhausted","lonely","cry","upset","depressed"});
    int fear = hits({"scared","afraid","worried","anxious","nervous"});
    int flirt = hits({"cute","beautiful","gorgeous","adorable","sexy","pretty","miss you","love you"});

    float v = (float)(joy + flirt) - (float)(anger + sad + fear);
    t.valence = std::min(std::max(v * 0.5f, -1.0f), 1.0f);
    t.warmth = std::min(joy * 0.3f + flirt * 0.4f + (low.find("thank") != std::string::npos ? 0.3f : 0.0f), 1.0f);
    t.flirt_signal = std::min(flirt * 0.5f + (msg.find("\xF0\x9F\x98\x8F") != std::string::npos ? 0.5f : 0.0f), 1.0f);

    auto cnt = [&](const char* w) { int n = 0; size_t p = 0; while ((p = low.find(w, p)) != std::string::npos) { n++; p += strlen(w); } return n; };
    int selfn = cnt(" i ") + cnt(" me ") + cnt(" my ");
    int youn = cnt(" you ") + cnt(" your ");
    t.self_focus = (selfn + youn) > 0 ? (float)selfn / (selfn + youn) : 0.5f;

    bool negword = anger > 0 || low.find("wrong") != std::string::npos || low.find(" not") != std::string::npos || low.find("no ") != std::string::npos;
    bool ather = low.find(" you") != std::string::npos;
    t.repair_signal = (negword && ather) ? std::max(0.5f, t.confidence) : 0.0f;

    float c = t.confidence;
    state_.emotions[0] = std::min(1.0f, state_.emotions[0] + 0.25f * joy * c);
    state_.emotions[1] = std::min(1.0f, state_.emotions[1] + 0.2f * flirt * c);
    state_.emotions[4] = std::min(1.0f, state_.emotions[4] + 0.25f * sad * c);
    state_.emotions[5] = std::min(1.0f, state_.emotions[5] + 0.25f * anger * c);
    state_.emotions[7] = std::min(1.0f, state_.emotions[7] + 0.25f * fear * c);

    // Sarcasm detection
    int sarc_hits = hits({"yeah right", "sure thing", "oh totally", "obviously", "right..."});
    bool has_laugh = low.find("lol") != std::string::npos || low.find("lmao") != std::string::npos;
    bool has_neg = anger > 0 || low.find("not") != std::string::npos || low.find("wrong") != std::string::npos;
    if ((sarc_hits > 0 || has_laugh) && has_neg && t.confidence > 0.3f) {
        t.sarcasm_signal = 1.0f;
    }
    return t;
}

std::string SoulCore::recall_query(const std::string& msg) {
    ensure_table();
    auto msg_words = tokenize(msg);
    if (msg_words.empty()) return "";
    sqlite3* db = nullptr;
    if (sqlite3_open_v2(DB, &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) { if (db) sqlite3_close(db); return ""; }
    sqlite3_stmt* st = nullptr;
    std::string best_prompt, best_emotions; float best_score = 0.0f;
    if (sqlite3_prepare_v2(db, "SELECT prompt, emotions, salience FROM episodes ORDER BY rowid DESC LIMIT 20;", -1, &st, nullptr) == SQLITE_OK) {
        while (sqlite3_step(st) == SQLITE_ROW) {
            const char* p = (const char*)sqlite3_column_text(st, 0);
            const char* e = (const char*)sqlite3_column_text(st, 1);
            float sal = (float)sqlite3_column_double(st, 2);
            if (!p) continue;
            auto words = tokenize(p);
            int shared = 0;
            for (auto& w : msg_words) if (words.count(w)) shared++;
            float score = shared + sal * 0.5f;
            if (shared >= 1 && score > best_score) { best_score = score; best_prompt = p; if (e) best_emotions = e; }
        }
        sqlite3_finalize(st);
    }
    sqlite3_close(db);
    if (best_prompt.empty()) return "";
    std::stringstream ss(best_emotions); std::string item; int i = 0;
    while (std::getline(ss, item, ',') && i < 8) {
        state_.emotions[i] = std::min(1.0f, state_.emotions[i] + 0.1f * (float)atof(item.c_str()));
        i++;
    }
    return "I remember when you said: " + best_prompt.substr(0, std::min((size_t)80, best_prompt.size()));
}


static std::string IM_START_USER() { return std::string("<|im_") + "start|>user"; }
static std::string IM_END() { return std::string("<|im_") + "end|>"; }

static std::string extract_user(const std::string& p) {
    size_t a = p.rfind(IM_START_USER());
    if (a == std::string::npos) return p;
    a = p.find('\n', a);
    if (a == std::string::npos) return p;
    a++;
    size_t b = p.find(IM_END(), a);
    if (b == std::string::npos) b = p.size();
    return p.substr(a, b - a);
}

void SoulCore::update(const std::string& user_msg, const std::string& task_class) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.interaction_count++;
    last_tone_ = tone_of(user_msg);
    last_recall_ = recall_query(user_msg);
    std::string state_json = state_to_json(last_recall_);
    std::string response = call_lua(state_json);
    if (!response.empty()) parse_lua_response(response);
    if (task_class == "code") {
        state_.styles.clear();
        state_.styles.push_back("locked_in");
        state_.tags.clear();
        state_.tags.push_back("precision");
        last_fragment_ = "[You are in locked_in mode: precise, technical, no jokes. Stay sharp and helpful.]";
    }

    // Intimacy Growth
    float vuln = state_.emotions[4] + state_.emotions[7]; // sadness + fear
    float share = last_tone_.self_focus * last_tone_.confidence;
    float growth = 0.005f + 0.02f * share + 0.01f * vuln;
    state_.intimacy = std::min(1.0f, state_.intimacy + growth);
    state_.last_interaction_time = time(nullptr);
    save_state();
    std::ofstream sl("/sdcard/Izanami/memory/soul_log.txt", std::ios::app);
    if (sl) sl << "update tone(a=" << last_tone_.arousal << ",v=" << last_tone_.valence << ",rep=" << last_tone_.repair_signal << ",fl=" << last_tone_.flirt_signal << ",conf=" << last_tone_.confidence << ") recall=" << last_recall_ << " | fragment=" << last_fragment_ << "\n";
}

std::string SoulCore::get_prompt_fragment() {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_fragment_;
}

void SoulCore::log_episode(const std::string& prompt, const std::string& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string raw = extract_user(prompt);
    float sal = std::max(salience_of(raw), salience_of(response));
    std::ostringstream em;
    for (int i = 0; i < 8; i++) em << state_.emotions[i] << (i < 7 ? "," : "");
    ensure_table();
    sqlite3* db = nullptr;
    if (sqlite3_open(DB, &db) == SQLITE_OK) {
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(db, "INSERT INTO episodes(prompt,response,emotions,salience,tags) VALUES(?,?,?,?,?);", -1, &st, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(st, 1, raw.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(st, 2, response.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(st, 3, em.str().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(st, 4, sal);
            sqlite3_bind_text(st, 5, "", -1, SQLITE_STATIC);
            sqlite3_step(st);
            sqlite3_finalize(st);
        }
        sqlite3_close(db);
    }
    std::ofstream ep("/sdcard/Izanami/memory/episodes_log.txt", std::ios::app);
    if (ep) ep << "---\n" << raw << "\n===\n" << response << "\n";
}

void SoulCore::decay(float hours_elapsed) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < 8; i++) {
        state_.emotions[i] *= (1.0f - 0.1f * hours_elapsed);
        if (state_.emotions[i] < 0.01f) state_.emotions[i] = 0.0f;
    }
}

std::string SoulCore::state_to_json(const std::string& recall) {
    std::ostringstream js;
    js << "{\"traits\":[";
    for (int i = 0; i < 6; i++) js << state_.traits[i] << (i < 5 ? "," : "");
    js << "],\"mood\":[";
    for (int i = 0; i < 3; i++) js << state_.mood[i] << (i < 2 ? "," : "");
    js << "],\"emotions\":[";
    for (int i = 0; i < 8; i++) js << state_.emotions[i] << (i < 7 ? "," : "");
    js << "],\"interaction_count\":" << state_.interaction_count;
    js << ",\"recall\":\"" << escape_json(recall) << "\"";
    time_t now = time(nullptr);
    int time_since = (int)difftime(now, state_.last_interaction_time);
    js << ",\"intimacy\":" << state_.intimacy << ",\"time_since\":" << time_since;
    js << ",\"tone\":{\"a\":" << last_tone_.arousal << ",\"v\":" << last_tone_.valence << ",\"w\":" << last_tone_.warmth << ",\"c\":" << last_tone_.confidence << ",\"fl\":" << last_tone_.flirt_signal << ",\"rep\":" << last_tone_.repair_signal << ",\"self\":" << last_tone_.self_focus << ",\"sarc\":" << last_tone_.sarcasm_signal << "}}";
    return js.str();
}

std::string SoulCore::call_lua(const std::string& state_json) {
    return LuaEngine::get_instance().execute_script("/sdcard/Izanami/soul.lua", state_json);
}

void SoulCore::parse_lua_response(const std::string& json) {
    size_t frag_start = json.find("\"fragment\":\"");
    if (frag_start != std::string::npos) {
        frag_start += 12;
        size_t frag_end = json.find("\"", frag_start);
        if (frag_end != std::string::npos) {
            last_fragment_ = json.substr(frag_start, frag_end - frag_start);
        }
    }
}


void SoulCore::load_state() {
    std::ifstream f("/sdcard/Izanami/memory/soul_state.txt");
    if (!f) return;
    std::string line;
    while (std::getline(f, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq);
        std::string v = line.substr(eq + 1);
        if (k == "intimacy") state_.intimacy = atof(v.c_str());
        else if (k == "last_time") state_.last_interaction_time = atol(v.c_str());
        else if (k == "base_caps") base_caps_ = atof(v.c_str());
        else if (k == "base_exclam") base_exclam_ = atof(v.c_str());
    }
}

void SoulCore::save_state() {
    std::ofstream f("/sdcard/Izanami/memory/soul_state.txt");
    if (!f) return;
    f << "intimacy=" << state_.intimacy << "\n";
    f << "last_time=" << state_.last_interaction_time << "\n";
    f << "base_caps=" << base_caps_ << "\n";
    f << "base_exclam=" << base_exclam_ << "\n";
}
