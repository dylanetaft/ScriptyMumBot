#pragma once

#include "MumBotEventListener.h"
#include "MumBotConnectionMgr.h"
#include <string>
#include <iostream>
#include <cstring>

//#define DEBUG1

#ifdef DEBUG0
#define D(x) x
#else
#define D(x)
#endif

#ifdef DEBUG1
#define D1(x) x
#else
#define D1(x)
#endif

std::string getVint(uint64_t val);
uint64_t readNextVint(std::string &data, uint32_t pos, uint32_t *finishpos);
