/*
 EasySFTP - Copyright (C) 2025 Kuri-Applications

 version.h - declaration of version string
 */

#pragma once

#include "../version-hash.h"

#define VERSION_NUMBER "0.13.0.1"

#define __W(x) L##x
#define _W(x) __W(x)
#define VERSION_STRING_W _W(VERSION_NUMBER) L"-" _W(VERSION_HASH)
