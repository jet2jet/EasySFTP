/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FileDisp.h - declaration of CFTPFileItemDisplayName
 */

#pragma once

class CFTPFileItemDisplayName : public CUnknownImplT<IDisplayItem>
{
public:
	// NOTE: pidl is released by CFTPFileItemDisplayName
	CFTPFileItemDisplayName(PIDLIST_ABSOLUTE pidl);
	virtual ~CFTPFileItemDisplayName();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD(GetItemIDList)(PIDLIST_ABSOLUTE* ppidl);
	STDMETHOD(GetItem)(IShellItem** ppsi);

private:
	PIDLIST_ABSOLUTE m_pidlMe;
};
