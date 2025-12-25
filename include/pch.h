#ifndef PCH_H
#define PCH_H

#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE_WORKSPACE_SIZE 512

#include "../libs/tree-sitter/lib/include/tree_sitter/api.h"
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <limits.h>
#include <magic.h>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <pcre2.h>
#include <queue>
#include <shared_mutex>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;
using namespace std::chrono_literals;

#endif
