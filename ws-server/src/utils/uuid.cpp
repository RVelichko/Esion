#include "uuid.hpp"


using namespace utils;

unsigned char RandomChar() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return static_cast<unsigned char>(dis(gen));
}


std::string GenerateHex(const unsigned int len) {
    std::stringstream ss;
    for (unsigned int i = 0; i < len; ++i) {
        auto rc = RandomChar();
        std::stringstream hexstream;
        hexstream << std::hex << int(rc);
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}
