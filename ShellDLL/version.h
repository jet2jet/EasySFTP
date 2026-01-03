/*
 EasySFTP - Copyright (C) 2025 jet (ジェット)

 version.h - declaration of version string
 */

#pragma once

#include "../version-hash.h"

#ifndef VERSION_STRING
#error VERSION_STRING is not defined
#endif

#define __W(x) L##x
#define _W(x) __W(x)
#define VERSION_STRING_W _W(VERSION_STRING) L"-" _W(VERSION_HASH)
