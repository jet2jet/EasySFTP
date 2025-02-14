/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 FoldDrop.cpp - implementations of drop handlers of folder-helper classes
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "FoldDrop.h"

#include "Folder.h"
#include "FoldRoot.h"
#include "FileStrm.h"

static ULONGLONG __stdcall GetFileSizeByName(LPCWSTR lpszFile)
{
	HANDLE h = ::MyCreateFileW(lpszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h && h != INVALID_HANDLE_VALUE)
	{
		ULARGE_INTEGER uli;
		uli.LowPart = ::GetFileSize(h, &uli.HighPart);
		if (uli.LowPart == 0xFFFFFFFF && ::GetLastError() != ERROR_SUCCESS)
			uli.QuadPart = 0;
		::CloseHandle(h);
		return uli.QuadPart;
	}
	return 0;
}

static HRESULT __stdcall SetPerformedDropEffect(IDataObject* pObject, DWORD dwEffect)
{
	FORMATETC fmt;
	STGMEDIUM stg;
	stg.hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
	if (!stg.hGlobal)
		return E_OUTOFMEMORY;

	fmt.cfFormat = theApp.m_nCFPerformedDropEffect;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.ptd = NULL;
	fmt.tymed = TYMED_HGLOBAL;
	stg.tymed = TYMED_HGLOBAL;
	stg.pUnkForRelease = NULL;
	LPDWORD lpdw = (LPDWORD) ::GlobalLock(stg.hGlobal);
	*lpdw = dwEffect;
	::GlobalUnlock(stg.hGlobal);
	HRESULT hr = pObject->SetData(&fmt, &stg, FALSE);
	::GlobalFree(stg.hGlobal);
	return hr;
}

static HRESULT __stdcall SetPasteSucceeded(IDataObject* pObject, DWORD dwEffect)
{
	FORMATETC fmt;
	STGMEDIUM stg;
	stg.hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
	if (!stg.hGlobal)
		return E_OUTOFMEMORY;

	fmt.cfFormat = theApp.m_nCFPasteSucceeded;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.ptd = NULL;
	fmt.tymed = TYMED_HGLOBAL;
	stg.tymed = TYMED_HGLOBAL;
	stg.pUnkForRelease = NULL;
	LPDWORD lpdw = (LPDWORD) ::GlobalLock(stg.hGlobal);
	*lpdw = dwEffect;
	::GlobalUnlock(stg.hGlobal);
	HRESULT hr = pObject->SetData(&fmt, &stg, FALSE);
	::GlobalFree(stg.hGlobal);
	return hr;
}

static HRESULT __stdcall GetRelativeDataObject(IShellFolder* pFolder, HWND hWndOwner, PCUIDLIST_RELATIVE pidlRelative, IDataObject** ppObject)
{
	if (IsSingleIDList(pidlRelative))
		return pFolder->GetUIObjectOf(hWndOwner, 1, (PCUITEMID_CHILD_ARRAY)&pidlRelative, IID_IDataObject, NULL, (void**)ppObject);
	IShellFolder* p2;
	PITEMID_CHILD pidlChild = ::GetChildItemIDList((PCUIDLIST_ABSOLUTE)pidlRelative);
	PIDLIST_RELATIVE pidlParent = (PIDLIST_RELATIVE) ::RemoveOneChild((PCUIDLIST_ABSOLUTE)pidlRelative);
	HRESULT hr = pFolder->BindToObject(pidlParent, NULL, IID_IShellFolder, (void**)&p2);
	if (SUCCEEDED(hr))
	{
		PCITEMID_CHILD pcidlChild = pidlChild;
		hr = p2->GetUIObjectOf(hWndOwner, 1, &pcidlChild, IID_IDataObject, NULL, (void**)ppObject);
	}
	::CoTaskMemFree(pidlParent);
	::CoTaskMemFree(pidlChild);
	return hr;
}

static HRESULT __stdcall RetrieveDirectoryRecursive(CMyPtrArrayT<CTransferDialog::CTransferItem>& aTransfers, CTransferDialog& dlgTransfer, const CMyStringW& strDirectory, LPCWSTR lpszBase)
{
	CMyStringW str, strBase, strRelative;

	::MyGetFileTitleStringW(strDirectory, str);
	if (lpszBase != NULL)
	{
		strBase = lpszBase;
		strBase += L'/';
	}
	strBase += str;
	CTransferDialog::CTransferItem* pvObject = dlgTransfer.AddTransferItem(0, strBase, strDirectory, true);
	aTransfers.Add(pvObject);

	str = strDirectory;
	str += L'\\';
	str += L'*';
	WIN32_FIND_DATAW wfd;
	auto handle = ::MyFindFirstFileW(str, &wfd);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return S_OK;
	}
	while (true)
	{
		if (wfd.cFileName[0] != L'.' || (wfd.cFileName[1] &&
			(wfd.cFileName[1] != L'.' || wfd.cFileName[2])))
		{
			::MyGetFullPathStringW(strDirectory, wfd.cFileName, str);

			if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				RetrieveDirectoryRecursive(aTransfers, dlgTransfer, str, strBase);
			}
			else
			{
				strRelative = strBase + L'/' + wfd.cFileName;
				ULONGLONG uliSize = GetFileSizeByName(str);
				CTransferDialog::CTransferItem* pvObject = dlgTransfer.AddTransferItem(uliSize, strRelative, str, true);
				aTransfers.Add(pvObject);
			}
		}
		if (!::MyFindNextFileW(handle, &wfd))
		{
			break;
		}
	}
	::FindClose(handle);
	return S_OK;
}

CFTPDropHandler::CFTPDropHandler(CFTPDirectoryBase* pDirectory, HWND hWndOwner)
	: m_uRef(1)
	, m_pDirectory(pDirectory)
	, m_hWndOwner(hWndOwner)
	, m_pObjectCur(NULL)
	, m_dwLastDragKeyState(0)
{
	pDirectory->AddRef();
}

CFTPDropHandler::~CFTPDropHandler()
{
	if (m_pObjectCur)
		m_pObjectCur->Release();
	m_pDirectory->Release();
}

STDMETHODIMP CFTPDropHandler::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IDropTarget))
	{
		*ppv = (IDropTarget*)this;
		AddRef();
		return S_OK;
	}
	return m_pDirectory->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CFTPDropHandler::AddRef()
{
	if (!m_uRef)
		return 0;
	return (ULONG) ::InterlockedIncrement((LONG*)&m_uRef);
}

STDMETHODIMP_(ULONG) CFTPDropHandler::Release()
{
	if (!m_uRef)
		return 0;
	ULONG u = (ULONG) ::InterlockedDecrement((LONG*)&m_uRef);
	if (u)
		return u;
	delete this;
	return 0;
}

STDMETHODIMP CFTPDropHandler::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	if (m_pObjectCur)
	{
		m_pObjectCur->Release();
		m_pObjectCur = NULL;
	}
	m_dwLastDragKeyState = 0;
	if (!pDataObj || !pdwEffect)
		return E_POINTER;

	m_pObjectCur = pDataObj;
	m_dwLastDragKeyState = grfKeyState;
	pDataObj->AddRef();

	DWORD dwEA, dwEP;
	dwEA = *pdwEffect;
	HRESULT hr = GetPreferredDropEffect(pDataObj, &dwEA, &dwEP);
	if (hr != S_OK)
		dwEP = DROPEFFECT_NONE;
	else if ((grfKeyState & MK_SHIFT) && (dwEA & DROPEFFECT_MOVE))
		dwEP = DROPEFFECT_MOVE;
	else if ((grfKeyState & MK_CONTROL) && (dwEA & DROPEFFECT_COPY))
		dwEP = DROPEFFECT_COPY;
	*pdwEffect = dwEP;

	return S_OK;
}

STDMETHODIMP CFTPDropHandler::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	if (!m_pObjectCur)
		return E_INVALIDARG;
	if (!pdwEffect)
		return E_POINTER;
	m_dwLastDragKeyState = grfKeyState;

	DWORD dwEA, dwEP;
	dwEA = *pdwEffect;
	dwEP = DROPEFFECT_NONE;
	HRESULT hr = GetPreferredDropEffect(m_pObjectCur, &dwEA, &dwEP);
	if (hr != S_OK)
		dwEP = DROPEFFECT_NONE;
	else if ((grfKeyState & MK_SHIFT) && (dwEA & DROPEFFECT_MOVE))
		dwEP = DROPEFFECT_MOVE;
	else if ((grfKeyState & MK_CONTROL) && (dwEA & DROPEFFECT_COPY))
		dwEP = DROPEFFECT_COPY;
	*pdwEffect = dwEP;

	return S_OK;
}

STDMETHODIMP CFTPDropHandler::DragLeave()
{
	if (m_pObjectCur)
	{
		m_pObjectCur->Release();
		m_pObjectCur = NULL;
	}
	return S_OK;
}

STDMETHODIMP CFTPDropHandler::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	if (!pdwEffect || !pDataObj)
		return E_POINTER;

	auto lastKeyState = m_dwLastDragKeyState;
	m_dwLastDragKeyState = 0;
	if (m_pObjectCur)
	{
		m_pObjectCur->Release();
		m_pObjectCur = NULL;
	}

	DWORD dwEA, dwEP;
	dwEA = *pdwEffect;
	dwEP = DROPEFFECT_NONE;
	HRESULT hr = GetPreferredDropEffect(pDataObj, &dwEA, &dwEP);
	if (hr != S_OK)
		dwEP = DROPEFFECT_NONE;
	else if ((grfKeyState & MK_SHIFT) && (dwEA & DROPEFFECT_MOVE))
		dwEP = DROPEFFECT_MOVE;
	else if ((grfKeyState & MK_CONTROL) && (dwEA & DROPEFFECT_COPY))
		dwEP = DROPEFFECT_COPY;

	if (dwEP == DROPEFFECT_NONE)
	{
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}

	if (lastKeyState & MK_RBUTTON)
	{
		HMENU hMenu = ::GetSubMenu(theApp.m_hMenuPopup, POPUP_POS_DROP);
		::EnableMenuItem(hMenu, ID_DROP_COPY, MF_BYCOMMAND | ((dwEA & DROPEFFECT_COPY) ? MF_ENABLED : MF_GRAYED));
		::EnableMenuItem(hMenu, ID_DROP_MOVE, MF_BYCOMMAND | ((dwEA & DROPEFFECT_MOVE) ? MF_ENABLED : MF_GRAYED));
		::EnableMenuItem(hMenu, ID_DROP_LINK, MF_BYCOMMAND | ((dwEA & DROPEFFECT_LINK) ? MF_ENABLED : MF_GRAYED));
		UINT uID = (UINT) ::TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y,
			0, m_hWndOwner, NULL);
		switch (uID)
		{
		case ID_DROP_COPY:
			dwEP = DROPEFFECT_COPY;
			break;
		case ID_DROP_MOVE:
			dwEP = DROPEFFECT_MOVE;
			break;
		case ID_DROP_LINK:
			dwEP = DROPEFFECT_LINK;
			break;
		default:
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
	}

	return PerformDrop(pDataObj, dwEP, m_pDirectory, m_hWndOwner, pdwEffect);
}

HRESULT CFTPDropHandler::PerformDrop(IDataObject* pDataObj, DWORD dwEffect, CFTPDirectoryBase* pDirectory, HWND hWndOwner, DWORD* pdwEffect)
{
	CFTPDropHandlerOperation* pOperation = new CFTPDropHandlerOperation(pDirectory, hWndOwner, pDataObj);
	if (!pOperation)
		return E_OUTOFMEMORY;

	pOperation->m_dwEffect = dwEffect;
	*pdwEffect = dwEffect;

	if (dwEffect != DROPEFFECT_NONE)
	{
		IAsyncOperation* pAsync = NULL;
		if (SUCCEEDED(pDataObj->QueryInterface(IID_IAsyncOperation, (void**)&pAsync)))
		{
			BOOL b = FALSE;
			if (FAILED(pAsync->SetAsyncMode(TRUE)) || FAILED(pAsync->GetAsyncMode(&b)) || !b)
			{
				pAsync->Release();
				pAsync = NULL;
			}
		}

		if (pAsync)
		{
			UINT u;
			HANDLE h = (HANDLE)_beginthreadex(NULL, 0,
				CFTPDropHandlerOperation::_ThreadProc,
				pOperation, 0, &u);
			if (!h || h == INVALID_HANDLE_VALUE)
			{
				pAsync->Release();
				delete pOperation;
				return E_UNEXPECTED;
			}
			::CloseHandle(h);
			if (FAILED(pAsync->StartOperation(NULL)))
			{
				pAsync->Release();
				pOperation->m_bFailedToStart = true;
				return E_UNEXPECTED;
			}
			else
				pOperation->m_pAsync = pAsync;
			pOperation->m_bStarted = true;
			return S_OK;
		}
		else
		{
			HRESULT hr = S_OK;
			FORMATETC fmt;
			fmt.cfFormat = CF_HDROP;
			fmt.dwAspect = DVASPECT_CONTENT;
			fmt.lindex = -1;
			fmt.ptd = NULL;
			fmt.tymed = TYMED_HGLOBAL;
			if (SUCCEEDED(pDataObj->QueryGetData(&fmt)))
			{
				STGMEDIUM stg;
				hr = pDataObj->GetData(&fmt, &stg);
				if (SUCCEEDED(hr))
				{
					pOperation->SetFileDropMode(&stg);
					*pdwEffect = DROPEFFECT_COPY;

					UINT u;
					HANDLE h = (HANDLE)_beginthreadex(NULL, 0,
						CFTPDropHandlerOperation::_ThreadProc,
						pOperation, 0, &u);
					if (!h || h == INVALID_HANDLE_VALUE)
					{
						delete pOperation;
						return E_UNEXPECTED;
					}
					::CloseHandle(h);
					pOperation->m_bStarted = true;
					return S_OK;
				}
			}
			hr = pOperation->DoOperation();
			delete pOperation;
			return hr;
		}
	}
	return S_OK;
}

HRESULT CFTPDropHandler::GetPreferredDropEffect(IDataObject* pObject, DWORD* pdwEffectAccept, DWORD* pdwEffectPrefer)
{
	if (!pObject || !pdwEffectAccept || !pdwEffectPrefer)
		return E_POINTER;

	FORMATETC fmt;
	HRESULT hr;
	bool bUni;
	DWORD dwE = *pdwEffectAccept;

	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.ptd = NULL;
	fmt.tymed = TYMED_HGLOBAL;
	fmt.cfFormat = theApp.m_nCFFTPData;
	hr = pObject->QueryGetData(&fmt);
	if (hr == S_OK)
	{
		STGMEDIUM stg;
		hr = pObject->GetData(&fmt, &stg);
		if (SUCCEEDED(hr))
		{
			LPCWSTR lpw = (LPCWSTR) ::GlobalLock(stg.hGlobal);
			if (m_pDirectory->GetRoot()->m_strHostName.Compare(lpw, true) == 0)
			{
				while (*lpw++);
				if (m_pDirectory->m_strDirectory.Compare(lpw, true) == 0)
					*pdwEffectAccept = DROPEFFECT_NONE;
				else
				{
					*pdwEffectAccept &= DROPEFFECT_COPY | DROPEFFECT_MOVE;
					*pdwEffectPrefer = DROPEFFECT_MOVE;
				}
			}
			else
			{
				*pdwEffectAccept &= DROPEFFECT_COPY | DROPEFFECT_MOVE;
				*pdwEffectPrefer = DROPEFFECT_COPY;
			}
			::GlobalUnlock(stg.hGlobal);
			::ReleaseStgMedium(&stg);
		}
		return S_OK;
	}
	fmt.cfFormat = theApp.m_nCFFileDescriptorW;
	hr = pObject->QueryGetData(&fmt);
	if (hr != S_OK)
	{
		bUni = false;
		fmt.cfFormat = theApp.m_nCFFileDescriptorA;
		hr = pObject->QueryGetData(&fmt);
	}
	else
		bUni = true;
	if (hr == S_OK)
	{
		fmt.tymed = TYMED_ISTREAM;
		fmt.cfFormat = theApp.m_nCFFileContents;
		hr = pObject->QueryGetData(&fmt);
		if (FAILED(hr))
			return hr;
		*pdwEffectAccept &= DROPEFFECT_COPY | DROPEFFECT_MOVE;
		*pdwEffectPrefer = DROPEFFECT_COPY;
		return S_OK;
	}

	*pdwEffectAccept &= DROPEFFECT_COPY | DROPEFFECT_MOVE;
	*pdwEffectPrefer = DROPEFFECT_COPY;
	fmt.cfFormat = theApp.m_nCFShellIDList;
	hr = pObject->QueryGetData(&fmt);
	if (hr == S_OK)
		return S_OK;
	fmt.cfFormat = CF_HDROP;
	hr = pObject->QueryGetData(&fmt);
	if (hr == S_OK)
		return S_OK;
	return S_FALSE;
}

////////////////////////////////////////////////////////////////////////////////

CFTPDropHandlerOperation::CFTPDropHandlerOperation(
	CFTPDirectoryBase* pDirectory, HWND hWndOwner, IDataObject* pObject, CTransferDialog* pDlgTransfer, bool bNoOwnDialog)
	: m_pDlgTransfer(pDlgTransfer)
{
	if (m_pDlgTransfer || bNoOwnDialog)
		m_bOwnDialog = false;
	else
	{
		m_pDlgTransfer = new CTransferDialog(this);
		m_bOwnDialog = true;
	}
	m_bStarted = false;
	m_bFailedToStart = false;
	m_pAsync = NULL;
	m_pDirectory = pDirectory;
	pDirectory->AddRef();
	m_hWndOwner = hWndOwner;
	m_pMarshalledObject = NULL;
	if (pObject)
		::CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pObject, &m_pMarshalledObject);
	m_bCanceled = false;
	m_bIsFileDrop = false;
}

CFTPDropHandlerOperation::~CFTPDropHandlerOperation()
{
	if (m_bIsFileDrop)
	{
		::ReleaseStgMedium(&m_stgFileData);
	}
	else
	{
		if (m_pAsync)
			m_pAsync->Release();
		if (m_pMarshalledObject)
			m_pMarshalledObject->Release();
	}
	m_pDirectory->Release();
	if (m_bOwnDialog)
		delete m_pDlgTransfer;
}

void CFTPDropHandlerOperation::TransferCanceled(void* pvTransfer)
{
	m_bCanceled = true;
}

void CFTPDropHandlerOperation::TransferInProgress(void* pvObject, ULONGLONG uliPosition)
{
	m_pDlgTransfer->UpdateTransferItem(static_cast<CTransferDialog::CTransferItem*>(pvObject), uliPosition);
}

bool CFTPDropHandlerOperation::TransferIsCanceled(void* pvObject)
{
	return m_bCanceled || m_pDirectory->GetRoot()->IsConnected() != S_OK;
}

void CFTPDropHandlerOperation::SetFileDropMode(const STGMEDIUM* pstg)
{
	if (!m_bIsFileDrop)
	{
		if (m_pAsync)
		{
			m_pAsync->Release();
			m_pAsync = NULL;
		}
		if (m_pMarshalledObject)
		{
			m_pMarshalledObject->Release();
			m_pMarshalledObject = NULL;
		}
	}
	memcpy(&m_stgFileData, pstg, sizeof(STGMEDIUM));
	m_bIsFileDrop = true;
}

HRESULT CFTPDropHandlerOperation::RetrieveFileContents(IDataObject* pObject, BYTE bTextMode)
{
	FORMATETC fmt;
	STGMEDIUM stg;
	HRESULT hr;
	bool bUni;

	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.ptd = NULL;
	fmt.tymed = TYMED_HGLOBAL;
	fmt.cfFormat = theApp.m_nCFFileDescriptorW;
	hr = pObject->QueryGetData(&fmt);
	if (hr != S_OK)
	{
		bUni = false;
		fmt.cfFormat = theApp.m_nCFFileDescriptorA;
		hr = pObject->QueryGetData(&fmt);
	}
	else
		bUni = true;
	if (hr == S_OK)
	{
		hr = pObject->GetData(&fmt, &stg);
		if (SUCCEEDED(hr))
		{
			union FILEGROUPDESCRIPTOR_UNION {
				FILEGROUPDESCRIPTORA a;
				FILEGROUPDESCRIPTORW w;
			} *pGroup;
			union FILEDESCRIPTOR_UNION {
				FILEDESCRIPTORA a;
				FILEDESCRIPTORW w;
			} *pDesc;
			CMyPtrArrayT<CTransferDialog::CTransferItem> aTransfers;
			pGroup = (FILEGROUPDESCRIPTOR_UNION*) ::GlobalLock(stg.hGlobal);
			fmt.cfFormat = theApp.m_nCFFileContents;
			fmt.tymed = TYMED_ISTREAM;

			auto* pRoot = m_pDirectory->GetRoot();
			pRoot->AddRef();
			m_bCanceled = false;
			if (m_bOwnDialog)
				m_pDlgTransfer->CreateW(m_hWndOwner);
			if (bUni)
				pDesc = (FILEDESCRIPTOR_UNION*)pGroup->w.fgd;
			else
				pDesc = (FILEDESCRIPTOR_UNION*)pGroup->a.fgd;
			for (fmt.lindex = 0; fmt.lindex < (LONG)pGroup->w.cItems; fmt.lindex++)
			{
				if ((bUni && !(pDesc->w.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) ||
					(!bUni && !(pDesc->a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
				{
					CMyStringW strName;
					if (bUni)
						strName = pDesc->w.cFileName;
					else
						strName = pDesc->a.cFileName;
					{
						CMyStringW str2;
						LPCWSTR lpw = wcsrchr(strName, L'\\');
						if (lpw)
						{
							str2 = lpw + 1;
							strName = str2;
						}
						lpw = wcsrchr(strName, L'/');
						if (lpw)
						{
							str2 = lpw + 1;
							strName = str2;
						}
					}
					ULARGE_INTEGER uliSize;
					if (bUni)
					{
						uliSize.LowPart = pDesc->w.nFileSizeLow;
						uliSize.HighPart = pDesc->w.nFileSizeHigh;
					}
					else
					{
						uliSize.LowPart = pDesc->a.nFileSizeLow;
						uliSize.HighPart = pDesc->a.nFileSizeHigh;
					}
					aTransfers.Add(m_pDlgTransfer->AddTransferItem(uliSize.QuadPart, strName, NULL, true));
				}
				else
					aTransfers.Add(NULL);
				if (bUni)
					pDesc = (FILEDESCRIPTOR_UNION*)(&pDesc->w + 1);
				else
					pDesc = (FILEDESCRIPTOR_UNION*)(&pDesc->a + 1);
			}

			if (bUni)
				pDesc = (FILEDESCRIPTOR_UNION*)pGroup->w.fgd;
			else
				pDesc = (FILEDESCRIPTOR_UNION*)pGroup->a.fgd;
			for (fmt.lindex = 0; fmt.lindex < (LONG)pGroup->w.cItems; fmt.lindex++)
			{
				CMyStringW strName;
				if (bUni)
					strName = pDesc->w.cFileName;
				else
					strName = pDesc->a.cFileName;
				{
					LPWSTR lpw = strName.GetBuffer();
					while (*lpw)
					{
						if (*lpw == L'\\')
							*lpw = L'/';
						lpw++;
					}
				}
				register bool bIsDir;
				if (bUni)
					bIsDir = (pDesc->w.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
				else
					bIsDir = (pDesc->a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
				if (bIsDir)
				{
					hr = pRoot->CreateFTPDirectory(m_hWndOwner, m_pDirectory, strName);
					if (FAILED(hr))
						break;
				}
				else
				{
					STGMEDIUM stg2;
					hr = pObject->GetData(&fmt, &stg2);
					if (hr == DV_E_LINDEX)
					{
						hr = S_OK;
						break;
					}
					else if (FAILED(hr))
						break;

					IStream* pStream = stg2.pstm;
					if (!TEXTMODE_IS_NO_CONVERTION(bTextMode))
					{
						IStream* pStream2;
						hr = m_pDirectory->IsTextFile(strName);
						hr = MyCreateTextStream(stg2.pstm,
							hr == S_OK ?
							TEXTMODE_FOR_SEND_LOCAL_STREAM(bTextMode) : 0,
							&pStream2);
						if (SUCCEEDED(hr))
							pStream = pStream2;
						else
							pStream->AddRef();
					}
					else
						pStream->AddRef();

					auto* pvObject = aTransfers.GetItem((int)fmt.lindex);
					hr = pRoot->WriteFTPItem(m_hWndOwner, m_pDirectory, strName, pStream,
						pvObject, this);
					pStream->Release();
					::ReleaseStgMedium(&stg2);
					if (FAILED(hr))
						break;
					if (m_bCanceled)
					{
						m_pDlgTransfer->RemoveTransferItem(pvObject, true);
						hr = E_ABORT;
						break;
					}
					m_pDlgTransfer->RemoveTransferItem(pvObject);

					//SUCCEEDED(hr) == true
					if (pRoot->m_bAdjustSendModifyTime)
					{
						CMyStringW str(m_pDirectory->m_strDirectory);
						if ((str.operator LPCWSTR())[str.GetLength() - 1] != L'/')
							str += L'/';
						str += strName;
						pRoot->SetFileTime(strName,
							NULL, NULL, bUni ? &pDesc->w.ftLastWriteTime : &pDesc->a.ftLastWriteTime);
					}
				}
				if (bUni)
					pDesc = (FILEDESCRIPTOR_UNION*)(&pDesc->w + 1);
				else
					pDesc = (FILEDESCRIPTOR_UNION*)(&pDesc->a + 1);
			}
			::ReleaseStgMedium(&stg);
			if (m_bOwnDialog)
				m_pDlgTransfer->DestroyWindow();
			pRoot->Release();
		}
	}
	else
	{
		fmt.tymed = TYMED_HGLOBAL;
		fmt.cfFormat = CF_HDROP;
		hr = pObject->QueryGetData(&fmt);
		if (hr == S_OK)
		{
			hr = pObject->GetData(&fmt, &stg);
			if (SUCCEEDED(hr))
			{
				hr = RetrieveFileName(stg.hGlobal, pObject);
				::ReleaseStgMedium(&stg);
			}
		}
	}

	return hr;
}

HRESULT CFTPDropHandlerOperation::RetrieveFileName(HGLOBAL hGlobal, IDataObject* pObject)
{
	HRESULT hr;
	LPDROPFILES lpdf = (LPDROPFILES) ::GlobalLock(hGlobal);
	LPBYTE lpb = (((LPBYTE)lpdf) + lpdf->pFiles);
	CMyStringW strFile, strName;
	CMyStringArrayW aFiles;
	CMyPtrArrayT<CTransferDialog::CTransferItem> aTransfers;

	m_bCanceled = false;
	if (m_bOwnDialog)
		m_pDlgTransfer->CreateW(m_hWndOwner);
	hr = S_OK;
	while (true)
	{
		if ((lpdf->fWide && !*((LPWSTR)lpb)) || (!lpdf->fWide && !*((LPSTR)lpb)))
			break;
		if (lpdf->fWide)
		{
			strFile = (LPCWSTR)lpb;
			while (*((LPWSTR&)lpb)++);
		}
		else
		{
			strFile = (LPCSTR)lpb;
			while (*((LPSTR&)lpb)++);
		}
		aFiles.Add(strFile);
	}
	::GlobalUnlock(hGlobal);
	BYTE bTextMode = m_pDirectory->GetRoot()->m_bTextMode;
	hr = RetrieveFileNameMultiple(aFiles, m_dwEffect == DROPEFFECT_MOVE, bTextMode);
	if (m_bOwnDialog)
		m_pDlgTransfer->DestroyWindow();
	if (SUCCEEDED(hr) && pObject)
		hr = SetPerformedDropEffect(pObject, DROPEFFECT_NONE);
	return hr;
}

HRESULT CFTPDropHandlerOperation::RetrieveFileNameSingle(LPCWSTR lpszFile, IStream* pStreamFile, LPCWSTR lpszDest, bool bIsMove, BYTE bTextMode)
{
	CMyPtrArrayT<CTransferDialog::CTransferItem> aTransfers;
	HRESULT hr = S_OK;
	if (pStreamFile)
	{
		STATSTG statstg = {};
		hr = pStreamFile->Stat(&statstg, STATFLAG_NONAME);
		if (FAILED(hr))
			statstg.cbSize.QuadPart = 0;
		auto* p = wcsrchr(lpszDest, L'/');
		if (!p)
			p = lpszDest;
		else
			++p;
		CTransferDialog::CTransferItem* pvObject = m_pDlgTransfer->AddTransferItem(statstg.cbSize.QuadPart, lpszDest, p, true);
		aTransfers.Add(pvObject);
	}
	else
	{
		if (::MyIsDirectoryW(lpszFile))
		{
			hr = RetrieveDirectoryRecursive(aTransfers, *m_pDlgTransfer, lpszFile, NULL);
			if (FAILED(hr))
				return hr;
		}
		else
		{
			ULONGLONG uliSize = GetFileSizeByName(lpszFile);
			CTransferDialog::CTransferItem* pvObject = m_pDlgTransfer->AddTransferItem(uliSize, lpszDest, lpszFile, true);
			aTransfers.Add(pvObject);
		}
	}
	CMyStringArrayW aDirectories;
	for (int nIndex = 0; nIndex < aTransfers.GetCount(); nIndex++)
	{
		CTransferDialog::CTransferItem* pvObject = aTransfers.GetItem(nIndex);
		hr = RetrieveFileNameImpl(pvObject, pvObject->strLocalFileName, pStreamFile, pvObject->strDestName, bIsMove, bTextMode, &aDirectories);
		if (FAILED(hr))
			break;
	}
	if (bIsMove)
	{
		for (int nIndex = 0; nIndex < aDirectories.GetCount(); nIndex++)
		{
			::MyRemoveDirectoryW(aDirectories.GetItem(nIndex));
		}
	}
	return hr;
}

HRESULT CFTPDropHandlerOperation::RetrieveFileNameMultiple(const CMyStringArrayW& aFiles, bool bIsMove, BYTE bTextMode)
{
	HRESULT hr;
	CMyPtrArrayT<CTransferDialog::CTransferItem> aTransfers;

	m_bCanceled = false;
	hr = S_OK;
	for (int i = 0; i < aFiles.GetCount(); ++i)
	{
		auto* pFile = aFiles.GetItem(i);
		auto* pName = wcsrchr(pFile, L'\\');
		if (!pName)
			pName = wcsrchr(pFile, L'/');
		if (!pName)
			pName = pFile;
		else
			++pName;
		if (::MyIsDirectoryW(pFile))
		{
			hr = RetrieveDirectoryRecursive(aTransfers, *m_pDlgTransfer, pFile, NULL);
			if (FAILED(hr))
			{
				break;
			}
		}
		else
		{
			ULONGLONG uliSize = GetFileSizeByName(pFile);
			CTransferDialog::CTransferItem* pvObject = m_pDlgTransfer->AddTransferItem(uliSize, pName, pFile, true);
			aTransfers.Add(pvObject);
		}
	}
	if (SUCCEEDED(hr))
	{
		CMyStringArrayW aDirectories;
		BYTE bTextMode = m_pDirectory->GetRoot()->m_bTextMode;
		for (int nIndex = 0; nIndex < aTransfers.GetCount(); nIndex++)
		{
			CTransferDialog::CTransferItem* pvObject = aTransfers.GetItem(nIndex);
			hr = RetrieveFileNameImpl(pvObject, pvObject->strLocalFileName, NULL, pvObject->strDestName, m_dwEffect == DROPEFFECT_MOVE, bTextMode, &aDirectories);
			if (FAILED(hr))
				break;
		}
		if (m_dwEffect == DROPEFFECT_MOVE)
		{
			for (int nIndex = 0; nIndex < aDirectories.GetCount(); nIndex++)
			{
				::MyRemoveDirectoryW(aDirectories.GetItem(nIndex));
			}
		}
	}
	return hr;
}

HRESULT CFTPDropHandlerOperation::RetrieveFileNameImpl(void* pvObject_, LPCWSTR lpszFile, IStream* pStreamFile, LPCWSTR lpszDest, bool bIsMove, BYTE bTextMode, CMyStringArrayW* paDirectories)
{
	HRESULT hr;
	auto* pvObject = static_cast<CTransferDialog::CTransferItem*>(pvObject_);
	auto isDir = pStreamFile ? false : ::MyIsDirectoryW(lpszFile);
	if (isDir && paDirectories)
	{
		paDirectories->Add(lpszFile);
		hr = m_pDirectory->GetRoot()->CreateFTPDirectory(m_hWndOwner, m_pDirectory, lpszDest);
		if (FAILED(hr))
			return hr;
	}
	else
	{
		IStream* pStream;
		if (pStreamFile)
		{
			hr = MyCreateTextStream(pStreamFile, bTextMode, &pStream);
		}
		else
		{
			hr = m_pDirectory->IsTextFile(lpszFile);
			if (hr != S_OK)
				bTextMode = TEXTMODE_NO_CONVERT;
			hr = MyOpenTextFileToStream(lpszFile, false,
				TEXTMODE_FOR_SEND_LOCAL_STREAM(bTextMode),
				&pStream);
		}
		if (FAILED(hr))
			return hr;
		hr = m_pDirectory->GetRoot()->WriteFTPItem(m_hWndOwner, m_pDirectory, lpszDest, pStream,
			pvObject, this);
		pStream->Release();
		if (FAILED(hr))
			return hr;
	}
	if (m_bCanceled)
	{
		m_pDlgTransfer->RemoveTransferItem(pvObject, true);
		return E_ABORT;
	}
	CMyStringW strFile = lpszFile ? lpszFile : L"";
	m_pDlgTransfer->RemoveTransferItem(pvObject);
	if (bIsMove)
	{
		if (!isDir)
		{
			::MyDeleteFileW(strFile);
		}
	}
	return S_OK;
}

HRESULT CFTPDropHandlerOperation::DoOperation()
{
	if (m_bIsFileDrop)
		return RetrieveFileName(m_stgFileData.hGlobal);
	//if (!m_pObject)
	//	return E_UNEXPECTED;
	IDataObject* pObject;
	HRESULT hr;
	hr = CoUnmarshalInterface(m_pMarshalledObject, IID_IDataObject, reinterpret_cast<void**>(&pObject));
	if (FAILED(hr))
		return hr;
	m_pMarshalledObject->Release();
	m_pMarshalledObject = NULL;

	FORMATETC fmt;
	STGMEDIUM stg;
	bool bFTPData;

	auto* pRoot = m_pDirectory->GetRoot();
	bFTPData = false;
	if (m_dwEffect == DROPEFFECT_MOVE)
	{
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.ptd = NULL;
		fmt.tymed = TYMED_HGLOBAL;
		fmt.cfFormat = theApp.m_nCFFTPData;
		hr = pObject->QueryGetData(&fmt);
		if (hr == S_OK)
		{
			hr = pObject->GetData(&fmt, &stg);
			if (SUCCEEDED(hr))
			{
				LPCWSTR lpw = (LPCWSTR) ::GlobalLock(stg.hGlobal);
				if (pRoot->m_strHostName.Compare(lpw, true) == 0)
				{
					// perform optimized move
					while (*lpw++);
					LPCWSTR lpszDir = lpw;
					while (*lpw++);
					bFTPData = true;
					hr = pRoot->MoveFTPItems(m_hWndOwner, m_pDirectory, lpszDir, lpw);
					if (SUCCEEDED(hr))
					{
						hr = SetPerformedDropEffect(pObject, DROPEFFECT_NONE);
						hr = SetPasteSucceeded(pObject, DROPEFFECT_NONE);
					}
				}
				::GlobalUnlock(stg.hGlobal);
				::ReleaseStgMedium(&stg);
			}
		}
	}
	if (!bFTPData)
	{
		hr = RetrieveFileContents(pObject, pRoot->m_bTextMode);
		if (hr == S_OK)
		{
			hr = SetPerformedDropEffect(pObject, m_dwEffect);
			hr = SetPasteSucceeded(pObject, m_dwEffect);
		}
		else if (hr == S_FALSE)
		{
			fmt.cfFormat = theApp.m_nCFShellIDList;
			fmt.dwAspect = DVASPECT_CONTENT;
			fmt.lindex = -1;
			fmt.ptd = NULL;
			fmt.tymed = TYMED_HGLOBAL;
			hr = pObject->QueryGetData(&fmt);
			if (hr == S_OK)
			{
				hr = pObject->GetData(&fmt, &stg);
				if (hr == S_OK)
				{
					LPIDA lpid = (LPIDA) ::GlobalLock(stg.hGlobal);
					if (lpid->cidl)
					{
						PCUIDLIST_ABSOLUTE pidlFolder = (PCUIDLIST_ABSOLUTE)
							(((LPBYTE)lpid) + lpid->aoffset[0]);
						IShellFolder* pFolder, * pDesktop;
						hr = ::SHGetDesktopFolder(&pDesktop);
						if (SUCCEEDED(hr))
						{
							if (IsDesktopIDList(pidlFolder))
							{
								pFolder = pDesktop;
								pFolder->AddRef();
							}
							else
								hr = pDesktop->BindToObject(pidlFolder, NULL, IID_IShellFolder, (void**)&pFolder);
							if (SUCCEEDED(hr))
							{
								IDataObject* pObj;
								for (UINT u = 1; u <= lpid->cidl; u++)
								{
									PCUIDLIST_RELATIVE pidlRelative = (PCUIDLIST_RELATIVE)
										(((LPBYTE)lpid) + lpid->aoffset[u]);
									hr = GetRelativeDataObject(pFolder, m_hWndOwner, pidlRelative, &pObj);
									if (SUCCEEDED(hr))
									{
										hr = RetrieveFileContents(pObj, pRoot->m_bTextMode);
										if (hr == S_OK)
										{
											hr = SetPerformedDropEffect(pObj, m_dwEffect);
											hr = SetPasteSucceeded(pObj, m_dwEffect);
										}
										pObj->Release();
										hr = SetPerformedDropEffect(pObject, m_dwEffect);
										hr = SetPasteSucceeded(pObject, m_dwEffect);
									}
								}
								pFolder->Release();
							}
							pDesktop->Release();
						}
					}
					::GlobalUnlock(stg.hGlobal);
					::ReleaseStgMedium(&stg);
				}
			}
			else
				hr = S_FALSE;
		}
	}

	if (m_pAsync)
	{
		hr = m_pAsync->EndOperation(hr, NULL, m_dwEffect);
		//m_pAsync->Release();
		//m_pAsync = NULL;
	}
	pObject->Release();
	//m_pObject->Release();
	//m_pObject = NULL;

	return hr;
}

UINT __stdcall CFTPDropHandlerOperation::_ThreadProc(void* pvArg)
{
	CFTPDropHandlerOperation* pThis = (CFTPDropHandlerOperation*)pvArg;

	::OleInitialize(NULL);
	while (!pThis->m_bStarted && !pThis->m_bFailedToStart)
		Sleep(1);
	if (!pThis->m_bFailedToStart)
		pThis->DoOperation();
	delete pThis;

	::OleUninitialize();
	_endthreadex(0);
	return 0;
}
