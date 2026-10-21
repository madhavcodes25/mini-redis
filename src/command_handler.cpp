#include "command_handler.hpp"
#include <cctype>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

std::vector<std::string> split(const std::string& line) {
    std::istringstream in(line);
    std::vector<std::string> tokens;
    std::string token;
    while (in >> token) tokens.push_back(token);
    return tokens;
}

std::string to_upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

bool parse_int(const std::string& s, int64_t& out) {
    try {
        size_t used = 0;
        long long v = std::stoll(s, &used);
        if (used != s.size()) return false;
        out = v;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string integer(int64_t n) { return "(integer) " + std::to_string(n); }

std::string wrong_args(const std::string& cmd) {
    return "ERR wrong number of arguments for '" + cmd + "' command";
}

const char* kNotInteger = "ERR value is not an integer or out of range";

}  // namespace

CommandHandler::CommandHandler(ExpiringStore& store) : store_(store) {}

std::string CommandHandler::execute(const std::string& line) {
    std::vector<std::string> t = split(line);
    if (t.empty()) return "";
    const std::string cmd = to_upper(t[0]);
    const size_t argc = t.size() - 1;

    if (cmd == "PING") {
        return argc == 0 ? "PONG" : wrong_args(cmd);
    }
    if (cmd == "SET") {
        if (argc != 2) return wrong_args(cmd);
        store_.set(t[1], t[2]);
        return "OK";
    }
    if (cmd == "GET") {
        if (argc != 1) return wrong_args(cmd);
        auto v = store_.get(t[1]);
        return v ? *v : "(nil)";
    }
    if (cmd == "DEL") {
        if (argc != 1) return wrong_args(cmd);
        return integer(store_.del(t[1]) ? 1 : 0);
    }
    if (cmd == "EXISTS") {
        if (argc != 1) return wrong_args(cmd);
        return integer(store_.get(t[1]) ? 1 : 0);
    }
    if (cmd == "INCR" || cmd == "DECR") {
        if (argc != 1) return wrong_args(cmd);
        const int64_t delta = (cmd == "INCR") ? 1 : -1;
        int64_t n = 0;
        auto current = store_.get(t[1]);
        if (current && !parse_int(*current, n)) return kNotInteger;
        if ((delta > 0 && n == INT64_MAX) || (delta < 0 && n == INT64_MIN)) {
            return "ERR increment or decrement would overflow";
        }
        n += delta;
        store_.set_keep_ttl(t[1], std::to_string(n));   // INCR must not wipe the key's TTL
        return integer(n);
    }
    if (cmd == "EXPIRE") {
        if (argc != 2) return wrong_args(cmd);
        int64_t seconds = 0;
        if (!parse_int(t[2], seconds)) return kNotInteger;
        return integer(store_.expire(t[1], seconds) ? 1 : 0);
    }
    if (cmd == "TTL") {
        if (argc != 1) return wrong_args(cmd);
        return integer(store_.ttl(t[1]));
    }
    if (cmd == "PERSIST") {
        if (argc != 1) return wrong_args(cmd);
        return integer(store_.persist(t[1]) ? 1 : 0);
    }
    if (cmd == "DBSIZE") {
        if (argc != 0) return wrong_args(cmd);
        return integer(static_cast<int64_t>(store_.size()));
    }
    return "ERR unknown command '" + t[0] + "'";
}