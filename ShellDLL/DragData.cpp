/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 DragData.cpp - implementations of classes for drag-and-drop
 */

#include "StdAfx.h"
#include "ShellDLL.h"
#include "FTPMsg.h"
#include "DragData.h"

#include "FileIcon.h"
#include "FTPFldr.h"
//#include "Transfer.h"
#include "SFTPFldr.h"
#include "SFTPStrm.h"

class CFTPStreamWrapper;

struct CFTPDataObjectFileStatus
{
	CFTPStreamWrapper* pWrapper;
	void* pvTransfer;

	CFTPDataObjectFileStatus() : pWrapper(NULL), pvTransfer(NULL) { }
};

////////////////////////////////////////////////////////////////////////////////

class CFTPStreamWrapper : public IStream
{
public:
	CFTPStreamWrapper(IStream* pStreamBase)
		: m_uRef(1)
		, m_pStreamBase(pStreamBase)
		//, m_bStopped(false)
		//, m_pDialog(NULL)
		//, m_pvTransfer(NULL)
	{
		m_uliOffsetDone.QuadPart = m_uliOffset.QuadPart = 0;
		STATSTG stg;
		stg.cbSize.QuadPart = 0;
		if (SUCCEEDED(m_pStreamBase->Stat(&stg, STATFLAG_NONAME)))
			m_uliMax.QuadPart = stg.cbSize.QuadPart;
		else
			m_uliMax.QuadPart = 0;
		m_pStreamBase->AddRef();
	}
	~CFTPStreamWrapper() { if (m_pStreamBase) m_pStreamBase->Release(); }

public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
	{
		if (IsEqualIID(riid, IID_IUnknown) ||
			IsEqualIID(riid, IID_ISequentialStream) ||
			IsEqualIID(riid, IID_IStream))
		{
			*ppvObject = (IStream*) this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return ++m_uRef; }
	STDMETHOD_(ULONG, Release)()
	{
		ULONG u = --m_uRef;
		if (!u)
			delete this;
		return u;
	}

	// ISequentialStream Interface
public:
	STDMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead)
	{
		if (!m_pStreamBase)
			return E_FAIL;
		ULONG ul = 0;
		HRESULT hr = m_pStreamBase->Read(pv, cb, &ul);
		/*if (hr == S_FALSE)
		{
			m_pDialog->RemoveTransferItem(m_pvTransfer);
			m_pvTransfer = NULL;
			m_pDialog = NULL;
		}
		else*/ if (hr == S_OK)
		{
			m_uliOffset.QuadPart += ul;
			m_uliOffsetDone.QuadPart = m_uliOffset.QuadPart;
			//m_pDialog->UpdateTransferItem(m_pvTransfer, m_uliOffset.QuadPart);
		}
		if (pcbRead)
			*pcbRead = ul;
		return hr;
	}
	STDMETHOD(Write)(void const* pv, ULONG cb, ULONG* pcbWritten)
	{
		if (!m_pStreamBase)
			return E_FAIL;
		ULONG ul = 0;
		HRESULT hr = m_pStreamBase->Write(pv, cb, &ul);
		if (hr == S_OK)
		{
			m_uliOffset.QuadPart += ul;
			m_uliOffsetDone.QuadPart = m_uliOffset.QuadPart;
			//m_pDialog->UpdateTransferItem(m_pvTransfer, m_uliOffset.QuadPart);
		}
		if (pcbWritten)
			*pcbWritten = ul;
		return hr;
	}

	// IStream Interface
public:
	STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize)
		{ return !m_pStreamBase ? E_FAIL : m_pStreamBase->SetSize(libNewSize); }
	STDMETHOD(CopyTo)(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten)
	{
		if (!pstm)
			return STG_E_INVALIDPOINTER;

		HRESULT hr = S_FALSE;
		ULONG ur, uw;
		CExBuffer buf;
		void* pv = buf.AppendToBuffer(NULL, 32768);
		if (!pv)
			return E_OUTOFMEMORY;
		while (cb.QuadPart)
		{
			hr = Read(pv, 32768, &ur);
			if (hr != S_OK)
			{
				if (SUCCEEDED(hr))
					hr = S_OK;
				break;
			}
			if (pcbRead)
				pcbRead->QuadPart += ur;
			hr = pstm->Write(pv, ur, &uw);
			if (FAILED(hr))
				break;
			if (pcbWritten)
				pcbWritten->QuadPart += uw;
		}
		return hr;
	}

	STDMETHOD(Commit)(DWORD grfCommitFlags)
		{ return !m_pStreamBase ? E_FAIL : m_pStreamBase->Commit(grfCommitFlags); }
	STDMETHOD(Revert)()
		{ return !m_pStreamBase ? E_FAIL : m_pStreamBase->Revert(); }
	STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
		{ return !m_pStreamBase ? E_FAIL : m_pStreamBase->LockRegion(libOffset, cb, dwLockType); }
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
		{ return !m_pStreamBase ? E_FAIL : m_pStreamBase->UnlockRegion(libOffset, cb, dwLockType); }
	STDMETHOD(Clone)(IStream** ppstm)
	{
		if (!m_pStreamBase)
			return E_FAIL;
		if (!ppstm)
			return E_POINTER;
		IStream* p;
		HRESULT hr = m_pStreamBase->Clone(&p);
		if (FAILED(hr))
			return hr;
		CFTPStreamWrapper* pw = new CFTPStreamWrapper(p);
		pw->m_uliOffset.QuadPart = m_uliOffset.QuadPart;
		pw->m_uliOffsetDone.QuadPart = m_uliOffsetDone.QuadPart;
		//pw->m_bStopped = m_bStopped;
		//pw->m_pDialog = m_pDialog;
		//pw->m_pvTransfer = m_pvTransfer;
		*ppstm = pw;
		p->Release();
		return S_OK;
	}
	STDMETHOD(Seek)(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
	{
		if (!m_pStreamBase)
			return E_FAIL;
		HRESULT hr = m_pStreamBase->Seek(liDistanceToMove, dwOrigin, &m_uliOffset);
		if (lpNewFilePointer)
			lpNewFilePointer->QuadPart = m_uliOffset.QuadPart;
		return hr;
	}
	STDMETHOD(Stat)(STATSTG* pStatstg, DWORD grfStatFlag)
	{
		HRESULT hr = !m_pStreamBase ? E_FAIL : m_pStreamBase->Stat(pStatstg, grfStatFlag);
		return hr;
	}

public:
	bool StopOperation()
	{
		//m_bStopped = true;
		//if (m_pvTransfer)
		//{
		//	m_pDialog->RemoveTransferItem(m_pvTransfer);
		//	m_pvTransfer = NULL;
		//	m_pDialog = NULL;
		//}
		m_pStreamBase->Release();
		m_pStreamBase = NULL;
		return m_uliOffsetDone.QuadPart >= m_uliMax.QuadPart;
	}
	//void SetTransfer(CTransferDialog* pDialog, void* pvTransfer)
	//	{ m_pDialog = pDialog; m_pvTransfer = pvTransfer; }
	//void UpdateTransfer(CTransferDialog* pDialog, void* pvTransfer)
	//	{ pDialog->UpdateTransferItem(pvTransfer, m_uliOffsetDone.QuadPart); }

private:
	ULONG m_uRef;
	IStream* m_pStreamBase;
	ULARGE_INTEGER m_uliOffset, m_uliOffsetDone;
	ULARGE_INTEGER m_uliMax;
	//bool m_bStopped;
	//CTransferDialog* m_pDialog;
	//void* m_pvTransfer;
};

////////////////////////////////////////////////////////////////////////////////

CFTPDropSource::CFTPDropSource()
{
}

CFTPDropSource::~CFTPDropSource()
{
}

STDMETHODIMP CFTPDropSource::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IDropSource))
	{
		*ppv = (IDropSource*) this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CFTPDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if (fEscapePressed || (grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == (MK_LBUTTON | MK_RBUTTON))
		return DRAGDROP_S_CANCEL;

	if ((grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == 0)
		return DRAGDROP_S_DROP;

	return S_OK;
}

STDMETHODIMP CFTPDropSource::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

////////////////////////////////////////////////////////////////////////////////

CFTPDataObject::CFTPDataObject(//IFTPDataObjectListener* pListener,
		IMalloc* pMalloc,
		PIDLIST_ABSOLUTE pidlBase,
		LPCWSTR lpszHostName,
		CFTPConnection* pConnection,
		CSFTPFolderFTP* pRoot,
		CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aFiles)
	: m_uRef(1)
	//, m_pListener(pListener)
	, m_pMalloc(pMalloc)
	, m_strHostName(lpszHostName)
	// DROPEFFECT_COPY に初期化 (CFSTR_PERFORMEDDROPEFFECT を呼び出さないターゲットに対する措置)
	, m_dwPerformedDropEffect(DROPEFFECT_COPY)
	, m_dwPreferredDropEffect(DROPEFFECT_NONE)
	, m_nCFPerformed(0)
	, m_bIsClipboardData(false)
	, m_bAsyncMode(false)
	, m_bInOperation(false)
	, m_bSFTPMode(false)
	, m_fTextMode(TEXTMODE_NO_CONVERT)
	, m_pConnection(pConnection)
	, m_pFTPRoot(pRoot)
	, m_pSFTPRoot(NULL)
	, m_pChannel(NULL)
	, m_pDirectory(pDirectory)
	, m_pHolder(NULL)
	, m_aFiles(aFiles)
	//, m_pDialog(NULL)
{
	//pListener->AddRef();
	pMalloc->AddRef();
	pDirectory->AddRef();
	pConnection->AddRef();
	pRoot->AddRef();
	m_pidlBase = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList((PCUIDLIST_RELATIVE) pidlBase);

	int i = m_aFiles.GetCount();
	while (i--)
	{
		m_aFiles.GetItem(i)->AddRef();
		//m_aInfoReceived.Add(false);
		//m_aFileStatus.Add(new CFTPDataObjectFileStatus());
	}
}

CFTPDataObject::CFTPDataObject(//IFTPDataObjectListener* pListener,
		IMalloc* pMalloc,
		PIDLIST_ABSOLUTE pidlBase,
		LPCWSTR lpszHostName,
		CSFTPFolderSFTP* pRoot,
		CSFTPChannel* pChannel,
		CFTPDirectoryBase* pDirectory,
		const CMyPtrArrayT<CFTPFileItem>& aFiles)
	: m_uRef(1)
	//, m_pListener(pListener)
	, m_pMalloc(pMalloc)
	, m_strHostName(lpszHostName)
	// DROPEFFECT_COPY に初期化 (CFSTR_PERFORMEDDROPEFFECT を呼び出さないターゲットに対する措置)
	, m_dwPerformedDropEffect(DROPEFFECT_COPY)
	, m_dwPreferredDropEffect(DROPEFFECT_NONE)
	, m_nCFPerformed(0)
	, m_bIsClipboardData(false)
	, m_bDeleted(false)
	, m_bAsyncMode(false)
	, m_bInOperation(false)
	, m_bSFTPMode(true)
	, m_fTextMode(TEXTMODE_NO_CONVERT)
	, m_pSFTPRoot(pRoot)
	, m_pChannel(pChannel)
	, m_pConnection(NULL)
	, m_pFTPRoot(NULL)
	, m_pDirectory(pDirectory)
	, m_pHolder(NULL)
	, m_aFiles(aFiles)
	//, m_pDialog(NULL)
{
	//pListener->AddRef();
	pMalloc->AddRef();
	pDirectory->AddRef();
	m_pidlBase = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList((PCUIDLIST_RELATIVE) pidlBase);

	pRoot->AddRef();
	pChannel->AddRef();
	int i = m_aFiles.GetCount();
	while (i--)
	{
		m_aFiles.GetItem(i)->AddRef();
		//m_aInfoReceived.Add(false);
		//m_aFileStatus.Add(new CFTPDataObjectFileStatus());
	}
}

CFTPDataObject::~CFTPDataObject()
{
	// m_nCFPerformed == 0 は何もデータが取得されてない状態
	if (m_nCFPerformed != 0 && m_nCFPerformed != theApp.m_nCFFTPData)
	{
		// CFSTR_PERFORMEDDROPEFFECT を呼び出すターゲットが -_MOVE を設定した場合は
		// unoptimized move が行われているのでこちらで削除する
		if (m_dwPerformedDropEffect == DROPEFFECT_MOVE && !m_bDeleted)
		{
			for (int i = 0; i < m_aAllFileData.GetCount(); i++)
			{
				CFileData* pData = m_aAllFileData.GetItem(i);
				pData->pDirectory->DeleteFTPItem(pData->pItem);
			}
		}
	}

	{
		register int c;
		c = m_aFileStatus.GetCount();
		while (c--)
		{
			CFTPDataObjectFileStatus* p = (CFTPDataObjectFileStatus*) m_aFileStatus.GetItem(c);
			if (p->pWrapper)
				p->pWrapper->Release();
			delete p;
		}
		c = m_aAllFileData.GetCount();
		while (c--)
			delete m_aAllFileData.GetItem(c);
		c = m_aFiles.GetCount();
		while (c--)
			m_aFiles.GetItem(c)->Release();
	}
	if (m_pDirectory)
		m_pDirectory->Release();
	if (m_pHolder)
		m_pHolder->Release();
	if (m_pChannel)
		m_pChannel->Release();
	if (m_pSFTPRoot)
		m_pSFTPRoot->Release();
	if (m_pFTPRoot)
		m_pFTPRoot->Release();
	if (m_pConnection)
		m_pConnection->Release();
	m_pMalloc->Release();
	::CoTaskMemFree(m_pidlBase);
	//m_pListener->Release();
}

void CFTPDataObject::SetTextMode(BYTE fTextMode)
{
	m_fTextMode = fTextMode;
}

//void CFTPDataObject::AddFilesToTransferDialog(CTransferDialog* pDialog)
//{
//	m_pDialog = pDialog;
//	for (int i = 0; i < m_aFiles.GetCount(); i++)
//	{
//		CFTPFileItem* pItem = m_aFiles.GetItem(i);
//		CFTPDataObjectFileStatus* pStatus = (CFTPDataObjectFileStatus*) m_aFileStatus.GetItem(i);
//		if (!pStatus->pvTransfer)
//			pStatus->pvTransfer = pDialog->AddTransferItem(pItem->uliSize.QuadPart, pItem->strFileName);
//	}
//}
//
//void CFTPDataObject::UpdateToTransferDialog()
//{
//	for (int i = 0; i < m_aFiles.GetCount(); i++)
//	{
//		CFTPFileItem* pItem = m_aFiles.GetItem(i);
//		CFTPDataObjectFileStatus* pStatus = (CFTPDataObjectFileStatus*) m_aFileStatus.GetItem(i);
//		if (pStatus->pWrapper && pStatus->pvTransfer)
//		{
//			pStatus->pWrapper->UpdateTransfer(m_pDialog, pStatus->pvTransfer);
//		}
//	}
//}

void CFTPDataObject::DoStopOperation(bool bForce)
{
	for (int i = 0; i < m_aFileStatus.GetCount(); i++)
	{
		bool bCanceled = bForce;
		CFTPDataObjectFileStatus* pStatus = (CFTPDataObjectFileStatus*) m_aFileStatus.GetItem(i);
		if (pStatus->pWrapper)
		{
			if (!pStatus->pWrapper->StopOperation())
				bCanceled = true;
			pStatus->pWrapper->Release();
			pStatus->pWrapper = NULL;
		}
		//if (pStatus->pvTransfer)
		//{
		//	m_pDialog->RemoveTransferItem(pStatus->pvTransfer, bCanceled);
		//	pStatus->pvTransfer = NULL;
		//}
	}
	//m_pDialog = NULL;
	m_bInOperation = false;
	if (bForce)
	{
		m_aFiles.RemoveAll();
		//m_aInfoReceived.RemoveAll();
		int i = m_aAllFileData.GetCount();
		while (i--)
			delete m_aAllFileData.GetItem(i);
		m_aAllFileData.RemoveAll();
		m_aFileStatus.RemoveAll();
	}
}

//bool CFTPDataObject::CancelOperation(void* pvTransfer)
//{
//	if (!pvTransfer)
//		return false;
//	for (int i = 0; i < m_aFileStatus.GetCount(); i++)
//	{
//		CFTPDataObjectFileStatus* pStatus = (CFTPDataObjectFileStatus*) m_aFileStatus.GetItem(i);
//		if (pStatus->pvTransfer == pvTransfer)
//		{
//			if (pStatus->pWrapper)
//			{
//				pStatus->pWrapper->StopOperation();
//				pStatus->pWrapper->Release();
//				pStatus->pWrapper = NULL;
//			}
//			m_pDialog->RemoveTransferItem(pvTransfer, true);
//			pStatus->pvTransfer = NULL;
//			m_aFileStatus.RemoveItem(i);
//			return true;
//		}
//	}
//	return false;
//}

STDMETHODIMP CFTPDataObject::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IDataObject))
	{
		*ppv = (IDataObject*) this;
	}
	else if (IsEqualIID(riid, IID_IAsyncOperation))
	{
		*ppv = (IAsyncOperation*) this;
		// set m_bAsyncMode to true for async operation
		m_bAsyncMode = true;
	}
	// called in Windows 7 (or others)
	else if (IsEqualIID(riid, IID_IExtractIconW) || IsEqualIID(riid, IID_IExtractIconA))
	{
		if (m_aFiles.GetCount() != 1)
			return E_NOINTERFACE;
		CFTPFileItemIcon* pIcon = new CFTPFileItemIcon(m_aFiles.GetItem(0));
		HRESULT hr = pIcon->QueryInterface(riid, ppv);
		pIcon->Release();
		return hr;
	}
	else
	{
#ifdef _DEBUG
		CMyStringW str;
		str.Format(L"CFTPDataObject::QueryInterface: unknown interface: {%08lX-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
			riid.Data1, riid.Data2, riid.Data3, (UINT) riid.Data4[0], (UINT) riid.Data4[1],
			(UINT) riid.Data4[2], (UINT) riid.Data4[3], (UINT) riid.Data4[4], (UINT) riid.Data4[5],
			(UINT) riid.Data4[6], (UINT) riid.Data4[7]);
		OutputDebugString(str);
#endif
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CFTPDataObject::AddRef()
{
//#ifdef _DEBUG
//	CMyStringW str;
//	str.Format(L"CFTPDataObject(0x%p): AddRef(): ref = %lu\n", this, m_uRef + 1);
//	OutputDebugString(str);
//#endif
	return ++m_uRef;
}

STDMETHODIMP_(ULONG) CFTPDataObject::Release()
{
//#ifdef _DEBUG
//	CMyStringW str;
//	str.Format(L"CFTPDataObject(0x%p): Release(): ref = %lu\n", this, m_uRef - 1);
//	OutputDebugString(str);
//#endif
	if (!--m_uRef)
	{
		delete this;
		return 0;
	}
	return m_uRef;
}

HRESULT CFTPDataObject::GetFileDescriptorCountAndInitFileList(
	LPCWSTR lpszRelativeDir,
	CFTPDirectoryBase* pDirectory,
	const CMyPtrArrayT<CFTPFileItem>& aFiles,
	void* pSyncMsg,
	UINT* puItems)
{
	int i;
	for (i = 0; i < aFiles.GetCount(); i++)
	{
		CFTPFileItem* pItem = aFiles.GetItem(i);

		//if (!m_aInfoReceived.GetItem(i))
		//{
			if (m_bSFTPMode)
			{
				if (!((CSFTPSyncMessenger*) pSyncMsg)->TryStat(pItem))
				{
					return E_FAIL;
				}
				pItem->uliSize.QuadPart = ((CSFTPSyncMessenger*) pSyncMsg)->m_uliDataSize.QuadPart;
				memcpy(&pItem->ftModifyTime, &((CSFTPSyncMessenger*) pSyncMsg)->m_ftModTime, sizeof(FILETIME));
			}
			else
			{
				CMyStringW str(m_pDirectory->m_strDirectory);
				if (str.IsEmpty() || ((LPCWSTR) str)[str.GetLength() - 1] != L'/')
					str += L'/';
				str += pItem->strFileName;
				if (m_pConnection->IsCommandAvailable(L"SIZE"))
				{
					CWaitFileInfoData* pWait = new CWaitFileInfoData(CWaitFileInfoData::fileInfoSize);
					pWait->strFileName = str;
					pWait->bSucceeded = false;
					pWait->bWaiting = true;
					m_pConnection->SendCommandWithType(L"SIZE", str, L"I", pWait);
					if (m_pFTPRoot->WaitForReceive(&pWait->bWaiting) && pWait->bSucceeded)
					{
						pItem->uliSize.QuadPart = pWait->uliSize.QuadPart;
					}
					delete pWait;
				}
				if (m_pConnection->IsCommandAvailable(L"MDTM"))
				{
					CWaitFileInfoData* pWait = new CWaitFileInfoData(CWaitFileInfoData::fileInfoMDTM);
					pWait->bSucceeded = false;
					pWait->bWaiting = true;
					m_pConnection->SendCommand(L"MDTM", str, pWait);
					if (m_pFTPRoot->WaitForReceive(&pWait->bWaiting) && pWait->bSucceeded)
					{
						memcpy(&pItem->ftModifyTime, &pWait->ftModifiedTime, sizeof(FILETIME));
					}
					delete pWait;
				}
			}
		//	m_aInfoReceived.SetItem(i, true);
		//}

		CFileData* pData = new CFileData();
		if (lpszRelativeDir)
		{
			pData->strRelativeFileName = lpszRelativeDir;
			pData->strRelativeFileName += L'\\';
		}
		pData->strRelativeFileName += pItem->strFileName;
		pData->pDirectory = pDirectory;
		pData->pItem = pItem;
		m_aAllFileData.Add(pData);

		m_aFileStatus.Add(new CFTPDataObjectFileStatus());

		if (pItem->IsDirectory() && !pItem->IsShortcut())
		{
			CFTPDirectoryBase* pDir;
			HRESULT hr = pDirectory->OpenNewDirectory(pItem->strFileName, &pDir);
			if (SUCCEEDED(hr))
			{
				if (!pDir->DoReceiveDirectory())
				{
					return E_FAIL;
				}
				CSFTPSyncMessenger* pSyncMsg2 = NULL;
				if (m_bSFTPMode)
					pSyncMsg2 = new CSFTPSyncMessenger(m_pSFTPRoot, m_pChannel, pDir->m_strDirectory);
				hr = GetFileDescriptorCountAndInitFileList(pData->strRelativeFileName,
					pDir, pDir->m_aFiles, pSyncMsg2, puItems);
				if (m_bSFTPMode)
					delete pSyncMsg2;
				if (FAILED(hr))
					return hr;
			}
		}
	}
	if (puItems)
		*puItems += (UINT) i;
	return S_OK;
}

STDMETHODIMP CFTPDataObject::GetData(FORMATETC* pfmtIn, STGMEDIUM* pmdm)
{
	HRESULT hr = QueryGetData(pfmtIn);
	if (FAILED(hr))
		return hr;
	// 既に接続から切り離されている場合はエラーを返す
	// (例: IAsyncOperation::EndOperation の後)
	if (m_bSFTPMode)
	{
		if (!m_pSFTPRoot || !m_pChannel)
			return OLE_E_NOTRUNNING;
	}
	else
	{
		if (!m_pConnection || !m_pFTPRoot)
			return OLE_E_NOTRUNNING;
	}
	hr = m_pDirectory->IsConnected();
	if (hr != S_OK)
		return OLE_E_NOTRUNNING;
	CFTPFileItem* pItem;
	int i;
	//if (pfmtIn->cfFormat == theApp.m_nCFFTPRenameFlag)
	if (pfmtIn->cfFormat == theApp.m_nCFPerformedDropEffect ||
		pfmtIn->cfFormat == theApp.m_nCFPreferredDropEffect)
	{
		HGLOBAL hglb;
		hglb = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
		if (!hglb)
			return STG_E_MEDIUMFULL;
		LPDWORD lpdw = (LPDWORD) ::GlobalLock(hglb);
		*lpdw = (pfmtIn->cfFormat == theApp.m_nCFPerformedDropEffect)
			? m_dwPerformedDropEffect : m_dwPreferredDropEffect;
		::GlobalUnlock(hglb);
		pmdm->hGlobal = hglb;
		pmdm->tymed = TYMED_HGLOBAL;
		pmdm->pUnkForRelease = NULL;
		return S_OK;
	}
	else if (pfmtIn->cfFormat == theApp.m_nCFPasteSucceeded)
		return E_UNEXPECTED;
	else if (pfmtIn->cfFormat == theApp.m_nCFFTPData)
	{
		size_t n;
		HGLOBAL hglb;

		n = sizeof(WCHAR) * (m_strHostName.GetLength() + 1);
		n += sizeof(WCHAR) * (m_pDirectory->m_strDirectory.GetLength() + 1);
		for (i = 0; i < m_aFiles.GetCount(); i++)
		{
			pItem = m_aFiles.GetItem(i);
			n += sizeof(WCHAR) * (pItem->strFileName.GetLength() + 1);
		}
		n += sizeof(WCHAR);
		hglb = ::GlobalAlloc(GMEM_MOVEABLE, n);
		if (!hglb)
			return STG_E_MEDIUMFULL;
		LPWSTR lpw = (LPWSTR) ::GlobalLock(hglb);
		n = sizeof(WCHAR) * (m_strHostName.GetLength() + 1);
		if (m_strHostName.IsEmpty())
			*lpw = 0;
		else
			memcpy(lpw, (LPCWSTR) m_strHostName, n);
		lpw = (LPWSTR) (((LPBYTE) lpw) + n);
		n = sizeof(WCHAR) * (m_pDirectory->m_strDirectory.GetLength() + 1);
		if (m_pDirectory->m_strDirectory.IsEmpty())
			*lpw = 0;
		else
			memcpy(lpw, (LPCWSTR) m_pDirectory->m_strDirectory, n);
		lpw = (LPWSTR) (((LPBYTE) lpw) + n);
		for (i = 0; i < m_aFiles.GetCount(); i++)
		{
			pItem = m_aFiles.GetItem(i);
			n = sizeof(WCHAR) * (pItem->strFileName.GetLength() + 1);
			if (pItem->strFileName.IsEmpty())
				*lpw = 0;
			else
				memcpy(lpw, (LPCWSTR) pItem->strFileName, n);
			lpw = (LPWSTR) (((LPBYTE) lpw) + n);
		}
		*lpw = 0;
		::GlobalUnlock(hglb);
		pmdm->hGlobal = hglb;
		pmdm->tymed = TYMED_HGLOBAL;
		pmdm->pUnkForRelease = NULL;
		//m_bAsyncMode = false;
		m_nCFPerformed = pfmtIn->cfFormat;
		return S_OK;
	}
	else if (pfmtIn->cfFormat == theApp.m_nCFShellIDList)
	{
		LPIDA lpida;
		LPBYTE lpbBuffer;
		CMySimpleArray<PITEMID_CHILD> aItemIDs;
		PITEMID_CHILD pidlChild;
		SIZE_T nSize;
		HGLOBAL hglb;

		nSize = sizeof(CIDA) + GetItemIDListSize(m_pidlBase);
		for (int i = 0; i < m_aFiles.GetCount(); i++)
		{
			pidlChild = ::CreateFileItem(m_pMalloc, m_aFiles.GetItem(i));
			if (!pidlChild)
			{
				while (i--)
					m_pMalloc->Free(aItemIDs.GetItem(i));
				return E_OUTOFMEMORY;
			}
			aItemIDs.Add(pidlChild);
			nSize += sizeof(UINT) + GetItemIDListSize(pidlChild);
		}

		hglb = ::GlobalAlloc(GMEM_MOVEABLE, nSize);
		if (!hglb)
			return STG_E_MEDIUMFULL;
		lpida = (LPIDA) ::GlobalLock(hglb);
		lpida->cidl = (UINT) aItemIDs.GetCount();
		lpida->aoffset[0] = sizeof(CIDA) + sizeof(UINT) * aItemIDs.GetCount();
		lpbBuffer = ((LPBYTE) lpida) + lpida->aoffset[0];
		nSize = GetItemIDListSize(m_pidlBase);
		memcpy(lpbBuffer, m_pidlBase, nSize);
		for (int i = 0; i < aItemIDs.GetCount(); i++)
		{
			lpida->aoffset[i + 1] = lpida->aoffset[i] + (UINT) nSize;
			lpbBuffer += nSize;
			pidlChild = aItemIDs.GetItem(i);
			nSize = GetItemIDListSize(pidlChild);
			memcpy(lpbBuffer, pidlChild, nSize);
			m_pMalloc->Free(pidlChild);
		}
		::GlobalUnlock(hglb);
		pmdm->hGlobal = hglb;
		pmdm->tymed = TYMED_HGLOBAL;
		pmdm->pUnkForRelease = NULL;
		return S_OK;
	}
	else if (pfmtIn->cfFormat == theApp.m_nCFFileContents)
	{
		IStream* pStream;
		int i = pfmtIn->lindex;
		CFTPDataObjectFileStatus* pStatus;
		//if (i < 0 || i >= m_aFiles.GetCount())
		if (!m_aAllFileData.GetCount())
		{
			CSFTPSyncMessenger* pSyncMsg;
			if (m_bSFTPMode)
				pSyncMsg = new CSFTPSyncMessenger(m_pSFTPRoot, m_pChannel, m_pDirectory->m_strDirectory);
			hr = GetFileDescriptorCountAndInitFileList(NULL, m_pDirectory, m_aFiles, pSyncMsg, NULL);
			if (m_bSFTPMode)
				delete pSyncMsg;
			if (FAILED(hr))
				return hr;
		}
		if (i < 0 || i >= m_aAllFileData.GetCount())
			return DV_E_LINDEX;
		//pItem = m_aFiles.GetItem(i);
		CFileData* pData = m_aAllFileData.GetItem(i);
		pItem = pData->pItem;
		pStatus = (CFTPDataObjectFileStatus*) m_aFileStatus.GetItem(i);
		if (!pStatus->pWrapper)
		{
			//if (m_bSFTPMode)
			//{
			//	hr = m_pListener->CreateStream(pItem, &pStream);
			//	if (FAILED(hr))
			//		return hr;
			//}
			//else
			//{
			//	//if (!m_pSocket->GetFTPFileSize(pItem->strFileName, &pItem->uliSize))
			//	//	return E_FAIL;

				//hr = m_pListener->CreateStream(pItem, &pStream);
				hr = pData->pDirectory->CreateStream(pItem, &pStream);

				if (FAILED(hr))
					return hr;
			//}

			CFTPStreamWrapper* pWrapper = new CFTPStreamWrapper(pStream);
			pStream->Release();
			//pStatus->pvTransfer = NULL;
			pStatus->pWrapper = pWrapper;
		}
		pStatus->pWrapper->AddRef();
		pmdm->pstm = pStatus->pWrapper;
		pmdm->tymed = TYMED_ISTREAM;
		pmdm->pUnkForRelease = NULL;
		m_nCFPerformed = pfmtIn->cfFormat;
		return S_OK;
	}
	else if (pfmtIn->cfFormat == theApp.m_nCFFileDescriptorA ||
		pfmtIn->cfFormat == theApp.m_nCFFileDescriptorW)
	{
		bool bUni = (pfmtIn->cfFormat == theApp.m_nCFFileDescriptorW);
		union FILEGROUPDESCRIPTOR_UNION {
			FILEGROUPDESCRIPTORA a;
			FILEGROUPDESCRIPTORW w;
		} * pGroup;
		union FILEDESCRIPTOR_UNION {
			FILEDESCRIPTORA a;
			FILEDESCRIPTORW w;
		} * pDesc;
		//int i;
		UINT i;
		UINT uCount;
		size_t nSize;
		HGLOBAL hglb;
		bool bFailed = false;

		nSize = CCSIZEOF_STRUCT(FILEGROUPDESCRIPTORW, cItems);
		if (!m_aAllFileData.GetCount())
		{
			CSFTPSyncMessenger* pSyncMsg;
			if (m_bSFTPMode)
				pSyncMsg = new CSFTPSyncMessenger(m_pSFTPRoot, m_pChannel, m_pDirectory->m_strDirectory);
			else
				pSyncMsg = NULL;
			uCount = 0;
			hr = GetFileDescriptorCountAndInitFileList(NULL, m_pDirectory, m_aFiles, pSyncMsg, &uCount);
			if (m_bSFTPMode)
				delete pSyncMsg;
			if (FAILED(hr))
				return hr;
		}
		else
			uCount = (UINT) m_aAllFileData.GetCount();
		nSize += ((SIZE_T) uCount) * (bUni ? sizeof(FILEDESCRIPTORW) : sizeof(FILEDESCRIPTORA));
		//for (i = 0; i < m_aFiles.GetCount(); i++)
		//{
		//	pItem = m_aFiles.GetItem(i);
		//	if (!m_aInfoReceived.GetItem(i))
		//	{
		//		if (m_bSFTPMode)
		//		{
		//			if (!pSyncMsg->TryStat(pItem))
		//			{
		//				bFailed = true;
		//				break;
		//			}
		//			pItem->uliSize.QuadPart = pSyncMsg->m_uliDataSize.QuadPart;
		//			memcpy(&pItem->ftModifyTime, &pSyncMsg->m_ftModTime, sizeof(FILETIME));
		//		}
		//		else
		//		{
		//			((CFTPSocket*) m_pSocket)->GetFTPFileSize(pItem->strFileName, &pItem->uliSize);
		//			((CFTPSocket*) m_pSocket)->GetFTPFileModifiedTime(pItem->strFileName, &pItem->ftModifyTime);
		//		}
		//		m_aInfoReceived.SetItem(i, true);
		//	}
		//	nSize += bUni ? sizeof(FILEDESCRIPTORW) : sizeof(FILEDESCRIPTORA);
		//}
		//if (bFailed)
		//	return E_FAIL;
		hglb = ::GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, nSize);
		if (!hglb)
			return STG_E_MEDIUMFULL;
		auto* pRoot = m_pDirectory->GetRoot();
		pGroup = (FILEGROUPDESCRIPTOR_UNION*) ::GlobalLock(hglb);
		pGroup->a.cItems = uCount;
		pDesc = (FILEDESCRIPTOR_UNION*) (((LPBYTE) pGroup) + CCSIZEOF_STRUCT(FILEGROUPDESCRIPTORW, cItems));
		for (i = 0; i < uCount; i++)
		{
			//pItem = m_aFiles.GetItem(i);
			CFileData* pData = m_aAllFileData.GetItem((int) i);
			pItem = pData->pItem;
			if (bUni)
			{
				pDesc->w.dwFlags = FD_ATTRIBUTES | FD_WRITESTIME | FD_PROGRESSUI | FD_UNICODE;
				if (TEXTMODE_IS_NO_CONVERTION(m_fTextMode) || pData->pDirectory->IsTextFile(pItem->strFileName) != S_OK)
					pDesc->w.dwFlags |= FD_FILESIZE;
				if (pRoot->m_bAdjustRecvModifyTime)
				{
					pDesc->w.dwFlags |= FD_CREATETIME;
					::GetSystemTimeAsFileTime(&pDesc->w.ftCreationTime);
					memcpy(&pDesc->w.ftLastWriteTime, &pDesc->w.ftCreationTime, sizeof(FILETIME));
				}
				else
				{
					if (pItem->ftCreateTime.dwLowDateTime != 0 && pItem->ftCreateTime.dwHighDateTime != 0)
					{
						pDesc->w.dwFlags |= FD_CREATETIME;
						memcpy(&pDesc->w.ftCreationTime, &pItem->ftCreateTime, sizeof(FILETIME));
					}
					memcpy(&pDesc->w.ftLastWriteTime, &pItem->ftModifyTime, sizeof(FILETIME));
				}
				pDesc->w.nFileSizeLow = pItem->uliSize.LowPart;
				pDesc->w.nFileSizeHigh = pItem->uliSize.HighPart;
				if (pItem->type == fitypeDir)
					pDesc->w.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
				else
					pDesc->w.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
				MyCopyStringLenW(pDesc->w.cFileName, pData->strRelativeFileName, MAX_PATH);
			}
			else
			{
				pDesc->a.dwFlags = FD_ATTRIBUTES | FD_WRITESTIME | FD_PROGRESSUI;
				if (TEXTMODE_IS_NO_CONVERTION(m_fTextMode) || pData->pDirectory->IsTextFile(pItem->strFileName) != S_OK)
					pDesc->a.dwFlags |= FD_FILESIZE;
				if (pItem->ftCreateTime.dwLowDateTime != 0 && pItem->ftCreateTime.dwHighDateTime != 0)
				{
					pDesc->a.dwFlags |= FD_CREATETIME;
					memcpy(&pDesc->a.ftCreationTime, &pItem->ftCreateTime, sizeof(FILETIME));
				}
				memcpy(&pDesc->a.ftCreationTime, &pItem->ftCreateTime, sizeof(FILETIME));
				memcpy(&pDesc->a.ftLastWriteTime, &pItem->ftModifyTime, sizeof(FILETIME));
				pDesc->a.nFileSizeLow = pItem->uliSize.LowPart;
				pDesc->a.nFileSizeHigh = pItem->uliSize.HighPart;
				if (pItem->type == fitypeDir)
					pDesc->a.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
				else
					pDesc->a.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
				::WideCharToMultiByte(CP_ACP, 0,
					pData->strRelativeFileName, (int) pData->strRelativeFileName.GetLength() + 1,
					pDesc->a.cFileName, MAX_PATH, NULL, NULL);
			}
			if (bUni)
				pDesc = (FILEDESCRIPTOR_UNION*) (((LPBYTE) pDesc) + sizeof(FILEDESCRIPTORW));
			else
				pDesc = (FILEDESCRIPTOR_UNION*) (((LPBYTE) pDesc) + sizeof(FILEDESCRIPTORA));
		}
		::GlobalUnlock(hglb);

		pmdm->hGlobal = hglb;
		pmdm->tymed = TYMED_HGLOBAL;
		pmdm->pUnkForRelease = NULL;
		return S_OK;
	}
	return DV_E_CLIPFORMAT;
}

STDMETHODIMP CFTPDataObject::GetDataHere(FORMATETC* pfmt, STGMEDIUM* pmdm)
{
	return E_NOTIMPL;
}

STDMETHODIMP CFTPDataObject::QueryGetData(FORMATETC* pfmt)
{
	if (!pfmt)
		return E_POINTER;
	for (int i = 0; i < theApp.m_aFTPDataFormats.GetCount(); i++)
	{
		FORMATETC* p = theApp.m_aFTPDataFormats.GetItemPtr(i);
		if (p->cfFormat == pfmt->cfFormat)
		{
			if (p->lindex < 0)
			{
				if (pfmt->lindex != -1)
					return DV_E_LINDEX;
			}
			else
			{
				if (m_aAllFileData.GetCount())
				{
					if (pfmt->lindex >= m_aAllFileData.GetCount())
						return DV_E_LINDEX;
				}
				else
				{
					if (pfmt->lindex >= m_aFiles.GetCount())
						return DV_E_LINDEX;
				}
			}
			if (pfmt->dwAspect != p->dwAspect)
				return DV_E_DVASPECT;
			if (!p->ptd && pfmt->ptd)
				return DV_E_FORMATETC;
			if (!(pfmt->tymed & p->tymed))
				return DV_E_TYMED;
			return S_OK;
		}
	}
	return DV_E_CLIPFORMAT;
}

STDMETHODIMP CFTPDataObject::GetCanonicalFormatEtc(FORMATETC* pfmtIn, FORMATETC* pfmtOut)
{
	HRESULT hr;

	hr = QueryGetData(pfmtIn);
	if (FAILED(hr))
		return hr;
	if (!pfmtOut)
		return E_POINTER;
	*pfmtOut = *pfmtIn;
	if (pfmtOut->ptd)
	{
		pfmtOut->ptd = NULL;
		return S_OK;
	}
	return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP CFTPDataObject::SetData(FORMATETC* pfmt, STGMEDIUM* pmdm, BOOL fRelease)
{
	if (!pfmt || !pmdm)
		return E_POINTER;
	//if (pfmt->cfFormat == theApp.m_nCFFTPRenameFlag)
	if (pfmt->cfFormat == theApp.m_nCFPerformedDropEffect)
	{
		if (pfmt->tymed != TYMED_HGLOBAL)
			return DV_E_TYMED;
		if (pfmt->lindex != -1)
			return DV_E_LINDEX;
		if (pfmt->dwAspect != DVASPECT_CONTENT)
			return DV_E_DVASPECT;
		if (pfmt->ptd != NULL)
			return DV_E_DVTARGETDEVICE;
		if (pmdm->tymed != TYMED_HGLOBAL)
			return DV_E_TYMED;
		LPDWORD lpdw = (LPDWORD) ::GlobalLock(pmdm->hGlobal);
		m_dwPerformedDropEffect = *lpdw;
		::GlobalUnlock(pmdm->hGlobal);
		if (fRelease)
			::ReleaseStgMedium(pmdm);
		return S_OK;
	}
	else if (pfmt->cfFormat == theApp.m_nCFPasteSucceeded)
	{
		if (pfmt->tymed != TYMED_HGLOBAL)
			return DV_E_TYMED;
		if (pfmt->lindex != -1)
			return DV_E_LINDEX;
		if (pfmt->dwAspect != DVASPECT_CONTENT)
			return DV_E_DVASPECT;
		if (pfmt->ptd != NULL)
			return DV_E_DVTARGETDEVICE;
		if (pmdm->tymed != TYMED_HGLOBAL)
			return DV_E_TYMED;
		LPDWORD lpdw = (LPDWORD) ::GlobalLock(pmdm->hGlobal);
		DWORD dwEffects = *lpdw;
		::GlobalUnlock(pmdm->hGlobal);
		if (fRelease)
			::ReleaseStgMedium(pmdm);

		if (!m_bAsyncMode || !m_bInOperation)
		{
			if (m_nCFPerformed != theApp.m_nCFFTPData)
			{
				// どちらも DROPEFFECT_MOVE の場合は unoptimized move
				if (dwEffects == DROPEFFECT_MOVE)
				{
					if (m_dwPerformedDropEffect == DROPEFFECT_MOVE)
					{
						for (int i = 0; i < m_aAllFileData.GetCount(); i++)
						{
							CFileData* pData = m_aAllFileData.GetItem(i);
							pData->pDirectory->DeleteFTPItem(pData->pItem);
						}
						m_bDeleted = true;
					}
				}
			}
		}
		m_pDirectory->AfterPaste(this, dwEffects);
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP CFTPDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppefe)
{
	if (!ppefe)
		return E_POINTER;
	*ppefe = new CFTPDataEnumFormatEtc(&theApp.m_aFTPDataFormats);
	return S_OK;
}

STDMETHODIMP CFTPDataObject::DAdvise(FORMATETC* pfmt, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
	if (!m_pHolder)
	{
		register HRESULT hr;
		hr = ::CreateDataAdviseHolder(&m_pHolder);
		if (FAILED(hr))
			return hr;
	}
	return m_pHolder->Advise(this, pfmt, advf, pAdvSink, pdwConnection);
}

STDMETHODIMP CFTPDataObject::DUnadvise(DWORD dwConnection)
{
	if (!m_pHolder)
		return OLE_E_NOCONNECTION;
	return m_pHolder->Unadvise(dwConnection);
}

STDMETHODIMP CFTPDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
	if (!m_pHolder)
	{
		register HRESULT hr;
		hr = ::CreateDataAdviseHolder(&m_pHolder);
		if (FAILED(hr))
			return hr;
	}
	return m_pHolder->EnumAdvise(ppenumAdvise);
}

STDMETHODIMP CFTPDataObject::SetAsyncMode(BOOL fDoOpAsync)
{
	if (fDoOpAsync && !m_bAsyncMode)
	{
		AddRef();
		m_bAsyncMode = true;
	}
	else if (!fDoOpAsync && m_bAsyncMode)
	{
		//Release();
		m_bAsyncMode = false;
	}
	return S_OK;
}

STDMETHODIMP CFTPDataObject::GetAsyncMode(BOOL* pfIsOpAsync)
{
	if (!pfIsOpAsync)
		return E_POINTER;
	*pfIsOpAsync = m_bAsyncMode ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CFTPDataObject::StartOperation(IBindCtx* pbcReserved)
{
	if (m_bSFTPMode)
	{
		if (!m_pSFTPRoot || !m_pChannel)
			return OLE_E_NOTRUNNING;
	}
	else
	{
		if (!m_pConnection || !m_pFTPRoot)
			return OLE_E_NOTRUNNING;
	}
	//theApp.m_aObjectTransferring.Add(this);
	m_bInOperation = true;
	//AddRef();
	return S_OK;
}

STDMETHODIMP CFTPDataObject::InOperation(BOOL* pfInAsyncOp)
{
	if (!pfInAsyncOp)
		return E_POINTER;
	*pfInAsyncOp = m_bInOperation ? TRUE : FALSE;
	return S_OK;
}

STDMETHODIMP CFTPDataObject::EndOperation(HRESULT hResult, IBindCtx* pbcReserved, DWORD dwEffects)
{
	if (m_bInOperation)
	{
		DoStopOperation();

		//{
		//	register int i = theApp.m_aObjectTransferring.FindItem(this);
		//	if (i >= 0)
		//		theApp.m_aObjectTransferring.RemoveItem(i);
		//}

		if (m_nCFPerformed != theApp.m_nCFFTPData)
		{
			// optimized move 用の SetData は EndOperation の前に呼び出される
			if (dwEffects == DROPEFFECT_MOVE)
			{
				if (m_dwPerformedDropEffect == DROPEFFECT_MOVE)
				{
					for (int i = 0; i < m_aAllFileData.GetCount(); i++)
					{
						CFileData* pData = m_aAllFileData.GetItem(i);
						pData->pDirectory->DeleteFTPItem(pData->pItem);
					}
					m_bDeleted = true;
				}
			}
		}
	}

	if (!m_bIsClipboardData)
	{
		// 接続を切り離す
		if (m_bSFTPMode)
		{
			if (m_pChannel)
			{
				m_pChannel->Release();
				m_pChannel = NULL;
			}
			if (m_pSFTPRoot)
			{
				m_pSFTPRoot->Release();
				m_pSFTPRoot = NULL;
			}
		}
		else
		{
			if (m_pConnection)
			{
				m_pConnection->Release();
				m_pConnection = NULL;
			}
			if (m_pFTPRoot)
			{
				m_pFTPRoot->Release();
				m_pFTPRoot = NULL;
			}
		}
	}
	//Release();
	return S_OK;
}
