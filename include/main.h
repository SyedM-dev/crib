#ifndef MAIN_H
#define MAIN_H

#include "pch.h"

#define NORMAL 0
#define INSERT 1
#define SELECT 2
#define RUNNER 3
#define JUMPER 4

extern std::atomic<bool> running;
extern uint8_t mode;

#endif
