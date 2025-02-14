/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FoldDrop.h - declarations of drop handlers of folder-helper classes
 */

#pragma once

#include "Transfer.h"
#include "TferStat.h"

class CFTPDirectoryBase;

class CFTPDropHandler : public IDropTarget
{
public:
	CFTPDropHandler(CFTPDirectoryBase* pDirectory, HWND hWndOwner);
	~CFTPDropHandler();

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	STDMETHOD(DragEnter)(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

	static HRESULT PerformDrop(IDataObject* pDataObj, DWORD dwEffect, CFTPDirectoryBase* pDirectory, HWND hWndOwner, DWORD* pdwEffect);

private:
	ULONG m_uRef;
protected:
	CFTPDirectoryBase* m_pDirectory;
	HWND m_hWndOwner;
	IDataObject* m_pObjectCur;
	DWORD m_dwLastDragKeyState;

	HRESULT GetPreferredDropEffect(IDataObject* pObject, DWORD* pdwEffectAccept, DWORD* pdwEffectPrefer);

	HRESULT ThreadProc();
};

class CFTPDropHandlerOperation : public CTransferDialogListener,
	public CTransferStatus
{
public:
	CFTPDropHandlerOperation(CFTPDirectoryBase* pDirectory, HWND hWndOwner, IDataObject* pObject, CTransferDialog* pDlgTransfer = NULL, bool bNoOwnDialog = false);
	~CFTPDropHandlerOperation();

	virtual void TransferCanceled(void* pvTransfer);
	virtual void TransferInProgress(void* pvObject, ULONGLONG uliPosition);
	virtual bool TransferIsCanceled(void* pvObject);

	void SetFileDropMode(const STGMEDIUM* pstg);

	HRESULT RetrieveFileContents(IDataObject* pObject, BYTE bTextMode);
	HRESULT RetrieveFileName(HGLOBAL hGlobal, IDataObject* pObject = NULL);
	HRESULT RetrieveFileNameSingle(LPCWSTR lpszFile, IStream* pStreamFile, LPCWSTR lpszDest, bool bIsMove, BYTE bTextMode);
	HRESULT RetrieveFileNameMultiple(const CMyStringArrayW& aFiles, bool bIsMove, BYTE bTextMode);
	HRESULT RetrieveFileNameImpl(void* pvObject, LPCWSTR lpszFile, IStream* pStreamFile, LPCWSTR lpszDest, bool bIsMove, BYTE bTextMode, CMyStringArrayW* paDirectories = NULL);
	HRESULT DoOperation();

	CFTPDirectoryBase* m_pDirectory;
	HWND m_hWndOwner;
	bool m_bIsFileDrop;
	union
	{
		struct
		{
			IStream* m_pMarshalledObject;
			IAsyncOperation* m_pAsync;
		};
		struct
		{
			STGMEDIUM m_stgFileData;
		};
	};
	DWORD m_dwEffect;
	bool m_bStarted;
	bool m_bFailedToStart;

	CTransferDialog* m_pDlgTransfer;
	bool m_bCanceled;
	bool m_bOwnDialog;

	// pvArg == (CFTPDropHandlerOperation*) pOperation
	static UINT __stdcall _ThreadProc(void* pvArg);
};
