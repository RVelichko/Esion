#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <string>

namespace utils {

unsigned char RandomChar();


std::string GenerateHex(const unsigned int len = 32);
} /// utils
