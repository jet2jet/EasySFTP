/*
 Copyright (C) 2025 Kuri-Applications

 ShlItem.h - declarations of utility functions for IShellItem
 */

#pragma once

STDAPI MyCreateShellItem(PCIDLIST_ABSOLUTE pidl, IShellItem** ppItem);
STDAPI MyCreateShellItemOfFolder(IShellFolder* pFolder, IShellItem** ppItem);
STDAPI MyCreateShellItemOnFolder(IShellFolder* pFolder, PCUIDLIST_RELATIVE pidl, IShellItem** ppItem);
STDAPI MyCreateShellItemFromRelativeName(IShellItem* pItemParent, LPCWSTR lpszRelativeName, IBindCtx* pb, IShellItem** ppItem);
