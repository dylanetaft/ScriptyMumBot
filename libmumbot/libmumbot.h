#pragma once

#include "MumBotEventListener.h"
#include "MumBotConnectionMgr.h"
#include <string>
#include <iostream>


#define DEBUG

#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif


std::string getVint(uint64_t val);
uint64_t readNextVint(std::string &data, uint32_t pos, uint32_t *finishpos);
