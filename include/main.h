#ifndef MAIN_H
#define MAIN_H

#include <atomic>
#include <vector>

#define NORMAL 0
#define INSERT 1
#define SELECT 2
#define RUNNER 3

extern std::atomic<bool> running;

#endif
