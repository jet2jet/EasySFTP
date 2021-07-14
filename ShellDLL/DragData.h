/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 DragData.h - declarations of classes for drag-and-drop
 */

#pragma once

#include "EnmArray.h"
#include "MySocket.h"
#include "FTPConn.h"
#include "FileList.h"
#include "SSHCli.h"
#include "SFTPChan.h"

#include "Unknown.h"

class CFTPDirectoryBase;
class CFTPDataObject;

class CSFTPFolderFTP;
class CSFTPFolderSFTP;

//class __declspec(novtable) IFTPDataObjectListener : public IUnknown
//{
//public:
//	virtual HRESULT CreateStream(CFTPFileItem* pItem, IStream** ppStream) = 0;
//	virtual void DeleteFTPItem(CFTPFileItem* pItem) = 0;
//	virtual void AfterPaste(CFTPDataObject* pObject, DWORD dwEffects) = 0;
//	STDMETHOD(IsTextFile)(LPCWSTR lpszFileName) = 0;
//};

//class __declspec(novtable) CFTPDataObjectListener : public CUnknownImplT<IFTPDataObjectListener>
//{
//public:
//	virtual HRESULT CreateStream(CFTPFileItem* pItem, IStream** ppStream) = 0;
//	virtual void DeleteFTPItem(CFTPFileItem* pItem) = 0;
//	virtual void AfterPaste(CFTPDataObject* pObject, DWORD dwEffects) = 0;
//	STDMETHOD(IsTextFile)(LPCWSTR lpszFileName) = 0;
//};

class CFTPDropSource : public CUnknownImplT<IDropSource>
{
public:
	CFTPDropSource();
	~CFTPDropSource();

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

	STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
	STDMETHOD(GiveFeedback)(DWORD dwEffect);
};

//class CTransferDialog;

struct CFileData
{
	CMyStringW strRelativeFileName;
	CFTPDirectoryBase* pDirectory;
	CFTPFileItem* pItem;
};

//class CFTPDataObject : public CUnknownImplT<IDataObject>
class CFTPDataObject : public IDataObject, public IAsyncOperation
{
public:
	CFTPDataObject(//IFTPDataObjectListener* pListener,
		IMalloc* pMalloc, PIDLIST_ABSOLUTE pidlBase,
		LPCWSTR lpszHostName, CFTPConnection* pConnection, CSFTPFolderFTP* pRoot, CFTPDirectoryBase* pDirectory, const CMyPtrArrayT<CFTPFileItem>& aFiles);
	CFTPDataObject(//IFTPDataObjectListener* pListener,
		IMalloc* pMalloc, PIDLIST_ABSOLUTE pidlBase,
		LPCWSTR lpszHostName, CSFTPFolderSFTP* pRoot, CSFTPChannel* pChannel, CFTPDirectoryBase* pDirectory, const CMyPtrArrayT<CFTPFileItem>& aFiles);
	~CFTPDataObject();

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	STDMETHOD(GetData)(FORMATETC* pfmtIn, STGMEDIUM* pmdm);
	STDMETHOD(GetDataHere)(FORMATETC* pfmt, STGMEDIUM* pmdm);
	STDMETHOD(QueryGetData)(FORMATETC* pfmt);
	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* pfmtIn, FORMATETC* pfmtOut);
	STDMETHOD(SetData)(FORMATETC* pfmt, STGMEDIUM* pmdm, BOOL fRelease);
	STDMETHOD(EnumFormatEtc)(DWORD dwDirection,  IEnumFORMATETC** ppefe);
	STDMETHOD(DAdvise)(FORMATETC* pfmt, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);
	STDMETHOD(DUnadvise)(DWORD dwConnection);
	STDMETHOD(EnumDAdvise)(IEnumSTATDATA** ppenumAdvise);

	STDMETHOD(SetAsyncMode)(BOOL fDoOpAsync);
	STDMETHOD(GetAsyncMode)(BOOL* pfIsOpAsync);
	STDMETHOD(StartOperation)(IBindCtx* pbcReserved);
	STDMETHOD(InOperation)(BOOL* pfInAsyncOp);
	STDMETHOD(EndOperation)(HRESULT hResult, IBindCtx* pbcReserved, DWORD dwEffects);

	void SetTextMode(BYTE fTextMode);

	//void AddFilesToTransferDialog(CTransferDialog* pDialog);
	//void UpdateToTransferDialog();
	void DoStopOperation(bool bForce = false);
	//bool CancelOperation(void* pvTransfer);

	// *pdwEffect == DROPEFFECT_MOVE and m_dwPerformedDropEffect == DROPEFFECT_MOVE (unoptimized move)
	//  ... renamed but not removed
	// *pdwEffect == DROPEFFECT_MOVE and m_dwPerformedDropEffect == DROPEFFECT_NONE (optimized move)
	//  ... renamed and removed from list
	DWORD m_dwPerformedDropEffect;
	// Move(Cut) operation or copy operation
	DWORD m_dwPreferredDropEffect;
	DWORD m_dwPasteSucceeded;
	UINT m_nCFPerformed;
	bool m_bIsClipboardData;
	bool m_bDeleted;

protected:
	HRESULT GetFileDescriptorCountAndInitFileList(LPCWSTR lpszRelativeDir,
		CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aFiles,
		void* pSyncMsg,
		UINT* puItems);

protected:
	ULONG m_uRef;
	bool m_bAsyncMode;
	bool m_bInOperation;

	bool m_bSFTPMode;
	//IFTPDataObjectListener* m_pListener;
	IMalloc* m_pMalloc;
	PIDLIST_ABSOLUTE m_pidlBase;
	CMyStringW m_strHostName;
	BYTE m_fTextMode;
	CFTPConnection* m_pConnection;
	CSFTPFolderFTP* m_pFTPRoot;
	CSFTPFolderSFTP* m_pSFTPRoot;
	CSFTPChannel* m_pChannel;
public:
	CFTPDirectoryBase* m_pDirectory;
	CMyPtrArrayT<CFTPFileItem> m_aFiles;
protected:
	CMyPtrArrayT<CFileData> m_aAllFileData;
	//CMySimpleArray<bool> m_aInfoReceived;
	CMyPtrArray m_aFileStatus;
	IDataAdviseHolder* m_pHolder;
	//CTransferDialog* m_pDialog;
};

typedef CEnumArrayT<IEnumFORMATETC, FORMATETC, const FORMATETC&> CFTPDataEnumFormatEtc;
