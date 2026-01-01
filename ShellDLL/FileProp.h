/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 FileProp.h - declaration of CFTPFileItemPropertyStore
 */

#pragma once

class CFTPDirectoryBase;
class CFTPFileItem;

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
