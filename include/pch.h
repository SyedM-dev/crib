#ifndef PCH_H
#define PCH_H

#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE_WORKSPACE_SIZE 512

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include <ruby.h>
#pragma clang diagnostic pop
#include <magic.h>
#include <nlohmann/json.hpp>
#include <pcre2.h>
extern "C" {
#include "libgrapheme/grapheme.h"
#include "unicode_width/unicode_width.h"
}
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <limits.h>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <set>
#include <shared_mutex>
#include <signal.h>
#include <stack>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;
using namespace std::chrono_literals;

#endif
