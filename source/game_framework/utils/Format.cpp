#include "Format.hpp"

namespace utils {

std::uint32_t strHash(const std::string& str, std::uint32_t hash) {
    return strHash(str.c_str(), hash);
}

float toFloat(const std::string& str) {
    return std::stof(str);
}

int toInt(const std::string& str) {
    return std::stoi(str);
}

bool toBool(const std::string& str) {
    switch (utils::strHash(str.c_str())) {
    case strHash("true"): // fall through
    case strHash("True"): // fall through
    case strHash("TRUE"):
        return true;
    default:
        return false;
    }
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;

    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }

    return elems;
}

} // namespace utils
