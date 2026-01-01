/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 TextStrm.h - declarations of text stream functions
 */

#pragma once

#include "TextMode.h"

STDAPI MyCreateTextStream(IStream* pStreamBase, BYTE fTextMode, IStream** ppStreamOut);
STDAPI MyOpenTextFileToStream(LPCWSTR pName, bool fWrite, BYTE fTextMode, IStream** ppStream);
STDAPI MyOpenTextFileToStreamEx(LPCWSTR lpFileName, DWORD dwDesiredAccess,
	DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, BYTE fTextMode, IStream** ppStream);
