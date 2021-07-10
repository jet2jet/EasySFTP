/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FoldDrop.h - declarations of drop handlers of folder-helper classes
 */

#pragma once

#include "Transfer.h"

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

private:
	ULONG m_uRef;
protected:
	CFTPDirectoryBase* m_pDirectory;
	HWND m_hWndOwner;
	IDataObject* m_pObjectCur;

	class CFTPDropHandlerOperation : public CTransferDialogListener,
		public CTransferStatus
	{
	public:
		CFTPDropHandlerOperation(CFTPDirectoryBase* pDirectory, HWND hWndOwner, IDataObject* pObject);
		~CFTPDropHandlerOperation();

		virtual void TransferCanceled(void* pvTransfer);
		virtual void TransferInProgress(void* pvObject, ULONGLONG uliPosition);
		virtual bool TransferIsCanceled(void* pvObject);

		void SetFileDropMode(const STGMEDIUM* pstg);

		HRESULT RetrieveFileContents(IDataObject* pObject);
		HRESULT RetrieveFileName(HGLOBAL hGlobal, IDataObject* pObject = NULL);
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

		CTransferDialog m_dlgTransfer;
		bool m_bCanceled;

		// pvArg == (CFTPDropHandlerOperation*) pOperation
		static UINT __stdcall _ThreadProc(void* pvArg);
	};

	HRESULT GetPreferredDropEffect(IDataObject* pObject, DWORD* pdwEffectAccept, DWORD* pdwEffectPrefer);

	HRESULT ThreadProc();
};
