/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FileStrm.h - declarations of file stream functions
 */

#pragma once

EXTERN_C const IID IID_IFileStream;

DECLARE_INTERFACE_(IFileStream, IStream)
{
	STDMETHOD(GetFileHandle)(HANDLE FAR* phFile) PURE;
};

STDAPI MyOpenFileToStream(LPCWSTR pName, bool fWrite, IStream** ppStream);
STDAPI MyOpenFileToStreamEx(LPCWSTR lpFileName, DWORD dwDesiredAccess,
	DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, IStream** ppStream);
STDAPI MyGetFileHandleFromStream(IStream* pStream, HANDLE* phFile);
