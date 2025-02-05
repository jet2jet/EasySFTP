/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FileProp.h - declaration of CFTPFileItemPropertyStore
 */

#pragma once

class CFTPDirectoryBase;
struct CFTPFileItem;

class CFTPFileItemPropertyStore : public CUnknownImplT<IPropertyStore>
{
public:
	CFTPFileItemPropertyStore(CFTPDirectoryBase* pDirectory, CFTPFileItem* pItem);
	virtual ~CFTPFileItemPropertyStore();

	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD(GetCount)(DWORD* cProps);
	STDMETHOD(GetAt)(DWORD iProp, PROPERTYKEY* pkey);
	STDMETHOD(GetValue)(REFPROPERTYKEY key, PROPVARIANT* pv);
	STDMETHOD(SetValue)(REFPROPERTYKEY key, REFPROPVARIANT propvar);
	STDMETHOD(Commit)(void);

private:
	CFTPDirectoryBase* m_pDirectory;
	CFTPFileItem* m_pItem;
};
