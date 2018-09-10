/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 PuTTYLib.h - declarations of functions for Pageant
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef const BYTE FAR* LPCBYTE;

int WINAPI PuTTYGetKeyList1(LPBYTE* ppKeyList);
int WINAPI PuTTYGetKeyList2(LPBYTE* ppKeyList);
void* PuTTYSignSSH2Key(LPCBYTE pszPubKey, LPCBYTE pszData, DWORD* pnOutLen);
void* WINAPI PuTTYHashSSH1Challenge(LPCBYTE pPubKey, DWORD nPubKeyLen, LPCBYTE pData, DWORD nDataLen, LPCSTR pszSessionId, DWORD* pnOutLen);
int WINAPI PuTTYGetSSH1KeyLen(LPBYTE pKey, int nMaxLen);
void WINAPI PuTTYFreeKeyList(LPBYTE pKeyList);

#ifdef __cplusplus
} // extern "C"
#endif
