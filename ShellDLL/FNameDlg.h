/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FNameDlg.h - declarations of FileNameDialog function
 */

#pragma once

bool __stdcall FileNameDialog(CMyStringW& rstrFileName, HWND hWndParent, bool bNoPath = true, bool bAllowWildcard = true);
