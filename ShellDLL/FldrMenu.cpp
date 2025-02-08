
#include "stdafx.h"
#include "ShellDLL.h"
#include "FldrMenu.h"

#include "Folder.h"
#include "FoldRoot.h"

#include "FNameDlg.h"
#include "LinkDlg.h"

#include "FoldDrop.h"

////////////////////////////////////////////////////////////////////////////////

CFTPFileItemMenu::CFTPFileItemMenu(CFTPDirectoryBase* pParent,
	PCIDLIST_ABSOLUTE pidlMe,
	IShellBrowser* pBrowser,
	const CMyPtrArrayT<CFTPFileItem>& aItems)
	: m_uRef(1)
	, m_pParent(pParent)
	, m_pBrowser(pBrowser)
	, m_aItems(aItems)
	, m_dlgTransfer(this)
{
	if (pParent)
		pParent->AddRef();
	if (pBrowser)
		pBrowser->AddRef();
	m_pUnkSite = NULL;
	m_pidlMe = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(pidlMe);
	m_nFileTypes = FFIM_FILETYPE_FILEITEM;
	for (int i = 0; i < aItems.GetCount(); i++)
	{
		char n = (aItems.GetItem(i)->IsDirectory() ? FFIM_FILETYPE_DIRECTORY : FFIM_FILETYPE_FILEITEM);
		if (i == 0)
			m_nFileTypes = n;
		else if (m_nFileTypes != n)
		{
			m_nFileTypes = FFIM_FILETYPE_COMPLEX;
			break;
		}
	}
}

CFTPFileItemMenu::~CFTPFileItemMenu()
{
	::CoTaskMemFree(m_pidlMe);
	if (m_pUnkSite)
		m_pUnkSite->Release();
	if (m_pBrowser)
		m_pBrowser->Release();
	if (m_pParent)
		m_pParent->Release();
}

STDMETHODIMP CFTPFileItemMenu::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;

	// NOTE: we must set NULL to *ppv, or causes 0xC000041D exception in Windows 7 (x64)
	*ppv = NULL;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IContextMenu))
	{
		*ppv = (IContextMenu*) this;
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IObjectWithSite))
	{
		*ppv = (IObjectWithSite*) this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CFTPFileItemMenu::AddRef()
{
	return (ULONG) ::InterlockedIncrement((LONG*) &m_uRef);
}

STDMETHODIMP_(ULONG) CFTPFileItemMenu::Release()
{
	ULONG u = (ULONG) ::InterlockedDecrement((LONG*) &m_uRef);
	if (!u)
		delete this;
	return u;
}

STDMETHODIMP CFTPFileItemMenu::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	int i, nCount;
	MENUITEMINFO mii;
	UINT uMaxID;
	CMyStringW str;

#ifdef _WIN64
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_STATE;
#else
	mii.cbSize = MENUITEMINFO_SIZE_V1;
	mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
#endif
	uMaxID = idCmdFirst;

	UINT uIDDefault;
	if (m_nFileTypes == FFIM_FILETYPE_FILEITEM || m_nFileTypes == FFIM_FILETYPE_COMPLEX)
	{
		uIDDefault = ID_ITEM_OPEN;
	}
	else
	{
		uIDDefault = (uFlags & CMF_EXPLORE) ? ID_ITEM_EXPLORE : ID_ITEM_OPEN;
	}

	HMENU h = ::GetSubMenu(theApp.m_hMenuContext, CXMENU_POPUP_FILEITEM);
	nCount = ::GetMenuItemCount(h);
	for (i = 0; i < nCount; i++)
	{
		mii.cch = MAX_PATH;
#ifdef _UNICODE
		mii.dwTypeData = str.GetBufferW(MAX_PATH);
#else
		mii.dwTypeData = str.GetBufferA(MAX_PATH);
#endif
		::GetMenuItemInfo(h, (UINT) i, TRUE, &mii);
		if (uFlags & CMF_DEFAULTONLY)
		{
			if (mii.wID != uIDDefault)
				continue;
		}
		switch (m_nFileTypes)
		{
			case FFIM_FILETYPE_DIRECTORY:
			case FFIM_FILETYPE_COMPLEX:
				if (mii.wID == ID_ITEM_OPEN_AS_TEXT)
					continue;
				if (m_nFileTypes == FFIM_FILETYPE_DIRECTORY)
					break;
			case FFIM_FILETYPE_FILEITEM:
				if (mii.wID == ID_ITEM_EXPLORE)
					continue;
				break;
		}
		if (uFlags & CMF_DVFILE)
		{
			if (mii.wID == ID_ITEM_RENAME || mii.wID == ID_ITEM_DELETE ||
				mii.wID == ID_ITEM_PROPERTY || mii.wID == ID_ITEM_CUT ||
				mii.wID == ID_ITEM_COPY)
				continue;
		}
		if (!(uFlags & CMF_CANRENAME))
		{
			if (mii.wID == ID_ITEM_RENAME)
				continue;
		}
		if (!(uFlags & CMF_NODEFAULT))
		{
			if (mii.wID == uIDDefault)
				mii.fState |= MFS_DEFAULT;
		}
		mii.wID = (WORD)((UINT) mii.wID - ID_ITEM_BASE + idCmdFirst);
		if (uMaxID < (UINT) mii.wID)
			uMaxID = (UINT) mii.wID;
		::InsertMenuItem(hMenu, indexMenu++, TRUE, &mii);
	}
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, uMaxID - idCmdFirst + 1);
}

STDMETHODIMP CFTPFileItemMenu::InvokeCommand(CMINVOKECOMMANDINFO* pici)
{
	CMINVOKECOMMANDINFOEX* piciex = (CMINVOKECOMMANDINFOEX*) pici;
	UINT uID;
	if (piciex->cbSize == sizeof(CMINVOKECOMMANDINFOEX) && (piciex->fMask & CMIC_MASK_UNICODE))
	{
		if (!HIWORD(piciex->lpVerbW))
		{
			if (piciex->lpVerbW)
				uID = LOWORD(piciex->lpVerbW) + ID_ITEM_BASE;
			else
				uID = LOWORD(piciex->lpVerb) + ID_ITEM_BASE;
		}
		else
		{
			if (_wcsicmp(piciex->lpVerbW, L"open") == 0)
				uID = ID_ITEM_OPEN;
			else if (_wcsicmp(piciex->lpVerbW, L"explore") == 0)
				uID = ID_ITEM_EXPLORE;
			else if (_wcsicmp(piciex->lpVerbW, L"cut") == 0)
				uID = ID_ITEM_CUT;
			else if (_wcsicmp(piciex->lpVerbW, L"copy") == 0)
				uID = ID_ITEM_COPY;
			else if (_wcsicmp(piciex->lpVerbW, L"rename") == 0)
				uID = ID_ITEM_RENAME;
			else if (_wcsicmp(piciex->lpVerbW, L"delete") == 0)
				uID = ID_ITEM_DELETE;
			else if (_wcsicmp(piciex->lpVerbW, L"properties") == 0)
				uID = ID_ITEM_PROPERTY;
			else
				return E_INVALIDARG;
		}
	}
	else
	{
		if (!HIWORD(piciex->lpVerb))
			uID = LOWORD(piciex->lpVerb) + ID_ITEM_BASE;
		else
		{
			if (_stricmp(piciex->lpVerb, "open") == 0)
				uID = ID_ITEM_OPEN;
			else if (_stricmp(piciex->lpVerb, "explore") == 0)
				uID = ID_ITEM_EXPLORE;
			else if (_stricmp(piciex->lpVerb, "cut") == 0)
				uID = ID_ITEM_CUT;
			else if (_stricmp(piciex->lpVerb, "copy") == 0)
				uID = ID_ITEM_COPY;
			else if (_stricmp(piciex->lpVerb, "rename") == 0)
				uID = ID_ITEM_RENAME;
			else if (_stricmp(piciex->lpVerb, "delete") == 0)
				uID = ID_ITEM_DELETE;
			else if (_stricmp(piciex->lpVerb, "properties") == 0)
				uID = ID_ITEM_PROPERTY;
			else
				return E_INVALIDARG;
		}
	}

	{
		HWND hWnd = piciex->hwnd;
		if (!hWnd && m_pBrowser)
			m_pBrowser->GetWindow(&hWnd);
		if (!hWnd && m_pUnkSite)
		{
			IOleWindow* pWindow;
			if (SUCCEEDED(m_pUnkSite->QueryInterface(&pWindow)))
			{
				pWindow->GetWindow(&hWnd);
				pWindow->Release();
			}
		}
		switch (uID)
		{
			case ID_ITEM_OPEN:
			case ID_ITEM_EXPLORE:
			case ID_ITEM_OPEN_AS_TEXT:
				DoOpen(hWnd, uID != ID_ITEM_OPEN,
					(piciex->fMask & CMIC_MASK_HOTKEY) ? piciex->dwHotKey : 0);
				break;
			case ID_ITEM_DELETE:
				DoDelete(hWnd);
				break;
			case ID_ITEM_CUT:
				DoCut(hWnd);
				break;
			case ID_ITEM_COPY:
				DoCopy(hWnd);
				break;
			case ID_ITEM_RENAME:
				//return E_NOTIMPL;
				break;
			case ID_ITEM_PROPERTY:
				DoProperty(hWnd);
				break;
			default:
				return E_INVALIDARG;
		}
	}
	return S_OK;
}

STDMETHODIMP CFTPFileItemMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax)
{
	idCmd += ID_ITEM_BASE;

	bool bUnicode = false;
	if (uType & GCS_UNICODE)
	{
		bUnicode = true;
		uType &= ~GCS_UNICODE;
	}
	CMyStringW str;
	switch (idCmd)
	{
		case ID_ITEM_OPEN:
			str = L"open";
			break;
		case ID_ITEM_EXPLORE:
			str = L"explore";
			break;
		case ID_ITEM_OPEN_AS_TEXT:
			str = L"opentext";
			break;
		case ID_ITEM_CUT:
			str = L"cut";
			break;
		case ID_ITEM_COPY:
			str = L"copy";
			break;
		case ID_ITEM_DELETE:
			str = L"delete";
			break;
		case ID_ITEM_RENAME:
			str = L"rename";
			break;
		case ID_ITEM_PROPERTY:
			str = L"properties";
			break;
		default:
			return E_NOTIMPL;
	}
	if (uType == GCS_VALIDATEA)
		return S_OK;
	else if (uType == GCS_HELPTEXTA)
	{
		if (!str.LoadString((UINT) idCmd))
			str.Empty();
	}
	UINT dw = (UINT) (bUnicode ? str.GetLength() : str.GetLengthA());
	if (cchMax > dw + 1)
		cchMax = dw + 1;
	if (bUnicode)
	{
		memcpy(pszName, (LPCWSTR) str, sizeof(WCHAR) * (cchMax - 1));
		((LPWSTR) pszName)[cchMax - 1] = 0;
	}
	else
	{
		memcpy(pszName, (LPCSTR) str, sizeof(CHAR) * (cchMax - 1));
		((LPSTR) pszName)[cchMax - 1] = 0;
	}
	return S_OK;
}

STDMETHODIMP CFTPFileItemMenu::SetSite(IUnknown* pUnkSite)
{
	if (m_pUnkSite)
		m_pUnkSite->Release();
	m_pUnkSite = pUnkSite;
	if (pUnkSite)
	{
		pUnkSite->AddRef();

		IShellBrowser* pBrowser = NULL;
		HRESULT hr = pUnkSite->QueryInterface(IID_IShellBrowser, (void**) &pBrowser);
		if (FAILED(hr))
		{
			IServiceProvider* pService = NULL;
			if (SUCCEEDED(pUnkSite->QueryInterface(IID_IServiceProvider, (void**)&pService)))
			{
				if (FAILED(pService->QueryService(SID_SInPlaceBrowser, IID_IShellBrowser, (void**)&pBrowser)))
				{
					if (FAILED(pService->QueryService(SID_SShellBrowser, IID_IShellBrowser, (void**)&pBrowser)))
					{
						if (FAILED(pService->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void**)&pBrowser)))
						{
							pBrowser = NULL;
						}
					}
				}
				pService->Release();
			}
		}
		if (m_pBrowser)
			m_pBrowser->Release();
		m_pBrowser = pBrowser;
	}
	return S_OK;
}

STDMETHODIMP CFTPFileItemMenu::GetSite(REFIID riid, void** ppvSite)
{
	if (!ppvSite)
		return E_POINTER;
	if (!m_pUnkSite)
	{
		*ppvSite = NULL;
		return E_FAIL;
	}
	return m_pUnkSite->QueryInterface(riid, ppvSite);
}

void CFTPFileItemMenu::DoOpen(HWND hWndOwner, bool bExtend, DWORD dwHotKey)
{
	IShellBrowser* pBrowser;
	IServiceProvider* pService;
	pBrowser = m_pBrowser;
	if (hWndOwner)
		pBrowser = (IShellBrowser*) ::SendMessage(hWndOwner, CWM_GETISHELLBROWSER, 0, 0);
	if (pBrowser)
	{
		if (SUCCEEDED(pBrowser->QueryInterface(IID_IServiceProvider, (void**) &pService)))
		{
			if (FAILED(pService->QueryService(SID_SInPlaceBrowser, IID_IShellBrowser, (void**) &pBrowser)))
			{
				if (FAILED(pService->QueryService(SID_SShellBrowser, IID_IShellBrowser, (void**) &pBrowser)))
				{
					if (FAILED(pService->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void**) &pBrowser)))
					{
						pBrowser = m_pBrowser;
						if (pBrowser)
							pBrowser->AddRef();
					}
				}
			}
			pService->Release();
		}
		else
			pBrowser->AddRef();
	}

	CMyPtrArrayT<CFTPFileItem> aItems;
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
	sei.lpVerb = (!bExtend ? _T("open") : _T("explore"));
	sei.lpClass = _T("Folder");
	if (!pBrowser)
	{
		for (int i = 0; i < m_aItems.GetCount(); i++)
		{
			CFTPFileItem* pItem = m_aItems.GetItem(i);
			if (pItem->IsDirectory())
			{
				PITEMID_CHILD pidlItem = ::CreateFileItem(m_pParent->m_pRoot->GetDelegateMalloc(), pItem);
				sei.lpIDList = ::AppendItemIDList(m_pidlMe, pidlItem);
				::ShellExecuteEx(&sei);
				::CoTaskMemFree(sei.lpIDList);
				::CoTaskMemFree(pidlItem);
			}
			else
			{
				pItem->AddRef();
				aItems.Add(pItem);
			}
		}
	}
	else
	{
		UINT wFlags = SBSP_RELATIVE;
		if (bExtend)
			wFlags |= SBSP_EXPLOREMODE;
		else
			wFlags |= SBSP_DEFMODE;
		if (dwHotKey & MK_CONTROL)
			wFlags |= SBSP_NEWBROWSER;
		else
			wFlags |= SBSP_DEFBROWSER;

		bool bFirst = true;
		for (int i = 0; i < m_aItems.GetCount(); i++)
		{
			CFTPFileItem* pItem = m_aItems.GetItem(i);
			if (pItem->IsDirectory())
			{
				PITEMID_CHILD pidlItem = ::CreateFileItem(m_pParent->m_pRoot->GetDelegateMalloc(), pItem);
				if (bFirst)
				{
					pBrowser->BrowseObject(pidlItem, wFlags);
					bFirst = false;
				}
				else
				{
					sei.lpIDList = ::AppendItemIDList(m_pidlMe, pidlItem);
					::ShellExecuteEx(&sei);
					::CoTaskMemFree(sei.lpIDList);
				}
				::CoTaskMemFree(pidlItem);
			}
			else
			{
				pItem->AddRef();
				aItems.Add(pItem);
			}
		}
		pBrowser->Release();
	}
	if (aItems.GetCount() > 0)
		DownloadAndOpenFiles(hWndOwner, bExtend, aItems);
}

void CFTPFileItemMenu::DoCutCopy(HWND hWndOwner, bool bCut)
{
	CFTPDataObject* pObject;
	HRESULT hr;
	hr = m_pParent->m_pRoot->GetFTPItemUIObjectOf(hWndOwner, m_pParent, m_aItems, &pObject);
	if (FAILED(hr))
		return;
	//m_pParent->m_pRoot->BeforeClipboardOperation(pObject);
	pObject->m_bIsClipboardData = true;
	pObject->SetAsyncMode(TRUE);
	pObject->m_dwPreferredDropEffect = (bCut ? DROPEFFECT_MOVE : DROPEFFECT_COPY);
	//hr = ::OleSetClipboard(pObject);
	////if (SUCCEEDED(hr))
	////	m_pParent->m_pRoot->AfterClipboardOperation(pObject);
	//pObject->Release();
	theApp.PlaceClipboardData(pObject);
	pObject->Release();
}

void CFTPFileItemMenu::DoProperty(HWND hWndOwner)
{
	CServerFilePropertyDialog dlg(m_aItems);
	dlg.m_strDirectory = m_pParent->m_strDirectory;
	m_pParent->m_pRoot->PreShowPropertyDialog(&dlg);
	if (dlg.ModalDialogW(hWndOwner) == IDOK)
	{
		bool* pb = (bool*) malloc(sizeof(bool) * dlg.m_aAttrs.GetCount());
		memset(pb, 0, sizeof(bool) * dlg.m_aAttrs.GetCount());
		m_pParent->m_pRoot->UpdateFTPItemAttributes(hWndOwner, m_pParent, &dlg, dlg.m_aAttrs, pb);
		// currently no error check
		free(pb);
	}
}

void CFTPFileItemMenu::DoDelete(HWND hWndOwner)
{
	CMyStringW str;

	if (!m_aItems.GetCount())
		return;
	if (m_aItems.GetCount() == 1)
	{
		CFTPFileItem* pItem = m_aItems.GetItem(0);
		if (pItem->IsDirectory())
			str.Format(IDS_DELETE_DIRECTORY, (LPCWSTR) pItem->strFileName);
		else
			str.Format(IDS_DELETE_FILE, (LPCWSTR) pItem->strFileName);
	}
	else
		str.Format(IDS_DELETE_MULTIPLE, m_aItems.GetCount());

	if (::MyMessageBoxW(hWndOwner, str, NULL, MB_ICONQUESTION | MB_YESNO) != IDYES)
		return;

	m_pParent->m_pRoot->DoDeleteFTPItems(hWndOwner, m_pParent, m_aItems);

	//for (int i = 0; i < m_aItems.GetCount(); i++)
	//{
	//	CFTPFileItem* pItem = m_aItems.GetItem(i);
	//	m_pParent->UpdateRemoveFile(pItem->strFileName, pItem->IsDirectory());
	//	//PITEMID_CHILD pidlItem = ::CreateFileItem(m_pParent->m_pRoot->GetDelegateMalloc(), pItem);
	//	//m_pParent->NotifyUpdate(pItem->IsDirectory() ? SHCNE_RMDIR : SHCNE_DELETE, pidlItem, NULL);
	//	//::CoTaskMemFree(pidlItem);
	//}
}

#define BUFFER_SIZE      32768

void CFTPFileItemMenu::DownloadAndOpenFiles(HWND hWndOwner, bool bAsText, const CMyPtrArrayT<CFTPFileItem>& aItems)
{
	m_aTransfers.RemoveAll();

	void* pvBuffer = malloc(BUFFER_SIZE);
	if (!pvBuffer)
		return;

	m_dlgTransfer.CreateW(hWndOwner);

	for (int i = 0; i < aItems.GetCount(); i++)
	{
		CFTPFileItem* pItem = aItems.GetItem(i);
		m_aTransfers.Add(m_dlgTransfer.AddTransferItem(0, pItem->strFileName, NULL, true));
	}

	bool bQuit = false;
	for (int i = 0; i < aItems.GetCount(); i++)
	{
		CFTPFileItem* pItem = aItems.GetItem(i);
		auto* pvTransfer = m_aTransfers.GetItem(i);
		if (!bQuit && pvTransfer)
		{
			IStream* pStream;
			CMyStringW strFileName;
			HRESULT hr = m_pParent->m_pRoot->CreateFTPItemStream(m_pParent, pItem, &pStream);
			if (SUCCEEDED(hr))
			{
				if (!TEXTMODE_IS_NO_CONVERTION(m_pParent->m_pRoot->m_bTextMode))
				{
					hr = bAsText ? S_OK : m_pParent->IsTextFile(pItem->strFileName);
					if (SUCCEEDED(hr) && hr == S_OK)
					{
						IStream* pstm2;
						hr = MyCreateTextStream(pStream, TEXTMODE_FOR_RECV(m_pParent->m_pRoot->m_bTextMode), &pstm2);
						if (SUCCEEDED(hr))
						{
							pStream->Release();
							pStream = pstm2;
						}
					}
				}

				theApp.GetTemporaryFileName(pItem->strFileName, strFileName);
				m_dlgTransfer.SetTransferItemLocalFileName(pvTransfer, strFileName);
				{
					STATSTG stg;
					stg.cbSize.QuadPart = 0;
					if (SUCCEEDED(pStream->Stat(&stg, STATFLAG_NONAME)) && stg.cbSize.QuadPart)
						m_dlgTransfer.SetTransferItemSize(pvTransfer, stg.cbSize.QuadPart);
				}

				HANDLE hFile = ::MyCreateFileW(strFileName, GENERIC_WRITE, FILE_SHARE_READ,
					NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					bool bOK = false;
					ULONGLONG uliPos = 0;
					while (true)
					{
						while (theApp.CheckQueueMessage())
						{
							if (!theApp.MyPumpMessage2())
							{
								bQuit = true;
								break;
							}
						}
						if (!m_aTransfers.GetItem(i))
						{
							pvTransfer = NULL;
							break;
						}
						ULONG ul = 0, ul2;
						hr = pStream->Read(pvBuffer, BUFFER_SIZE, &ul);
						if (FAILED(hr))
							break;
						if (!ul)
						{
							bOK = true;
							break;
						}
						if (!::WriteFile(hFile, pvBuffer, ul, &ul2, NULL) || ul != ul2)
							break;
						uliPos += ul;
						m_dlgTransfer.UpdateTransferItem(pvTransfer, uliPos);
					}
					if (pvTransfer)
						m_dlgTransfer.RemoveTransferItem(pvTransfer, !bOK);
					::CloseHandle(hFile);
					if (bOK)
					{
						CMyStringW str;
						PITEMID_CHILD pidl = ::CreateFileItem(m_pParent->m_pMallocData->pMalloc, pItem);
						if (pidl)
						{
							STRRET strret;
							strret.uType = STRRET_WSTR;
							if (SUCCEEDED(m_pParent->GetDisplayNameOf(pidl, SHGDN_NORMAL | SHGDN_FORPARSING, &strret)))
							{
								switch (strret.uType)
								{
									case STRRET_WSTR:
										str = strret.pOleStr;
										::CoTaskMemFree(strret.pOleStr);
										break;
									case STRRET_CSTR:
										str = strret.cStr;
										break;
									case STRRET_OFFSET:
										str = (LPCSTR)(((LPCBYTE)pidl) + strret.uOffset);
										break;
								}
							}
							::CoTaskMemFree(pidl);
						}
						// 'str' can be empty
						theApp.SetAttachmentLock(strFileName, str);
						if (bAsText)
						{
							str = strFileName;
							str.InsertChar(L'\"', 0);
							str.AppendChar(L'\"');
							::MyShellExecuteWithParameterW(hWndOwner, L"notepad.exe", str);
						}
						else
							::MyShellOpenW(hWndOwner, strFileName);
					}
				}
				else
					m_dlgTransfer.RemoveTransferItem(pvTransfer, true);

				pStream->Release();
			}
		}
		pItem->Release();
	}

	m_dlgTransfer.DestroyWindow();

	free(pvBuffer);
}

void CFTPFileItemMenu::TransferCanceled(void* pvTransfer)
{
	if (pvTransfer)
	{
		int i = m_aTransfers.FindItem(static_cast<CTransferDialog::CTransferItem*>(pvTransfer));
		if (i >= 0)
			m_aTransfers.SetItem(i, NULL);
	}
	else
	{
		for (int i = 0; i < m_aTransfers.GetCount(); i++)
			m_aTransfers.SetItem(i, NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////

CFTPFileDirectoryMenu::CFTPFileDirectoryMenu(CFTPDirectoryBase* pParent,
	PCIDLIST_ABSOLUTE pidlMe,
	IShellBrowser* pBrowser)
	: m_uRef(1)
	, m_pParent(pParent)
	, m_pBrowser(pBrowser)
{
	if (pParent)
		pParent->AddRef();
	if (pBrowser)
		pBrowser->AddRef();
	m_pUnkSite = NULL;
	m_pidlMe = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(pidlMe);
}

CFTPFileDirectoryMenu::~CFTPFileDirectoryMenu()
{
	::CoTaskMemFree(m_pidlMe);
	if (m_pUnkSite)
		m_pUnkSite->Release();
	if (m_pBrowser)
		m_pBrowser->Release();
	if (m_pParent)
		m_pParent->Release();
}

STDMETHODIMP CFTPFileDirectoryMenu::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;

	// NOTE: we must set NULL to *ppv, or causes 0xC000041D exception in Windows 7 (x64)
	*ppv = NULL;

	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IContextMenu))
	{
		*ppv = (IContextMenu*) this;
		AddRef();
		return S_OK;
	}
	else if (IsEqualIID(riid, IID_IObjectWithSite))
	{
		*ppv = (IObjectWithSite*) this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CFTPFileDirectoryMenu::AddRef()
{
	return (ULONG) ::InterlockedIncrement((LONG*) &m_uRef);
}

STDMETHODIMP_(ULONG) CFTPFileDirectoryMenu::Release()
{
	ULONG u = (ULONG) ::InterlockedDecrement((LONG*) &m_uRef);
	if (!u)
		delete this;
	return u;
}

STDMETHODIMP CFTPFileDirectoryMenu::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	HMENU h = ::GetSubMenu(theApp.m_hMenuContext, CXMENU_POPUP_FILEDIR);
	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, _QueryContextMenu(hMenu, h, indexMenu, idCmdFirst, idCmdLast, uFlags) - idCmdFirst + 1);
}

UINT CFTPFileDirectoryMenu::_QueryContextMenu(HMENU hMenuTarget, HMENU hMenuCurrent, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	int i, nCount;
	MENUITEMINFO mii;
	UINT uMaxID;
	CMyStringW str;

#ifdef _WIN64
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
#else
	mii.cbSize = MENUITEMINFO_SIZE_V1;
	mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
#endif
	uMaxID = idCmdFirst;

	nCount = ::GetMenuItemCount(hMenuCurrent);
	for (i = 0; i < nCount; i++)
	{
		mii.cch = MAX_PATH;
#ifdef _UNICODE
		mii.dwTypeData = str.GetBufferW(MAX_PATH);
#else
		mii.dwTypeData = str.GetBufferA(MAX_PATH);
#endif
		::GetMenuItemInfo(hMenuCurrent, (UINT) i, TRUE, &mii);
		if (uFlags & CMF_DEFAULTONLY)
		{
			//if ((!(uFlags & CMF_EXPLORE) && mii.wID != ID_ITEM_OPEN) ||
			//	((uFlags & CMF_EXPLORE) && mii.wID != ID_ITEM_EXPLORE))
				continue;
		}
		if (mii.wID == ID_PARENT_NEW_SHORTCUT)
		{
			if (!m_pParent->m_pRoot->PreShowCreateShortcutDialog(NULL))
				continue;
		}
		//if (uFlags & CMF_DVFILE)
		//{
		//	if (mii.wID == ID_ITEM_RENAME || mii.wID == ID_ITEM_DELETE ||
		//		mii.wID == ID_ITEM_PROPERTY || mii.wID == ID_ITEM_CUT ||
		//		mii.wID == ID_ITEM_COPY)
		//		continue;
		//}
		//if (!(uFlags & CMF_CANRENAME))
		//{
		//	if (mii.wID == ID_ITEM_RENAME)
		//		continue;
		//}
		//if (!(uFlags & CMF_NODEFAULT))
		//{
		//	if ((!(uFlags & CMF_EXPLORE) && mii.wID == ID_ITEM_OPEN) ||
		//		((uFlags & CMF_EXPLORE) && mii.wID == ID_ITEM_EXPLORE))
		//		mii.fState |= MFS_DEFAULT;
		//}
		mii.wID = (WORD)((UINT) mii.wID - ID_PARENT_BASE + idCmdFirst);
		if (uMaxID < (UINT) mii.wID)
			uMaxID = (UINT) mii.wID;
		if (mii.hSubMenu)
		{
			HMENU h = CreatePopupMenu();
			UINT u = _QueryContextMenu(h, mii.hSubMenu, 0, idCmdFirst, idCmdLast, uFlags);
			// ignore 'u'
			mii.hSubMenu = h;
		}
		::InsertMenuItem(hMenuTarget, indexMenu++, TRUE, &mii);
	}
	return uMaxID;
}

STDMETHODIMP CFTPFileDirectoryMenu::InvokeCommand(CMINVOKECOMMANDINFO* pici)
{
	CMINVOKECOMMANDINFOEX* piciex = (CMINVOKECOMMANDINFOEX*) pici;
	UINT uID;
	if (piciex->cbSize == sizeof(CMINVOKECOMMANDINFOEX) && (piciex->fMask & CMIC_MASK_UNICODE))
	{
		if (!HIWORD(piciex->lpVerbW))
		{
			if (piciex->lpVerbW)
				uID = LOWORD(piciex->lpVerbW) + ID_PARENT_BASE;
			else
				uID = LOWORD(piciex->lpVerb) + ID_PARENT_BASE;
		}
		else
		{
			if (_wcsicmp(piciex->lpVerbW, L"newfolder") == 0)
				uID = ID_PARENT_NEW_FOLDER;
			else if (_wcsicmp(piciex->lpVerbW, L"newlink") == 0)
				uID = ID_PARENT_NEW_SHORTCUT;
			else if (_wcsicmp(piciex->lpVerbW, L"properties") == 0)
				uID = ID_PARENT_PROPERTY;
			else if (_wcsicmp(piciex->lpVerbW, L"paste") == 0)
				uID = ID_PARENT_PASTE;
			else
				return E_INVALIDARG;
		}
	}
	else
	{
		if (!HIWORD(piciex->lpVerb))
			uID = LOWORD(piciex->lpVerb) + ID_PARENT_BASE;
		else
		{
			if (_stricmp(piciex->lpVerb, "newfolder") == 0)
				uID = ID_PARENT_NEW_FOLDER;
			else if (_stricmp(piciex->lpVerb, "newlink") == 0)
				uID = ID_PARENT_NEW_SHORTCUT;
			else if (_stricmp(piciex->lpVerb, "properties") == 0)
				uID = ID_PARENT_PROPERTY;
			else if (_stricmp(piciex->lpVerb, "paste") == 0)
				uID = ID_PARENT_PASTE;
			else
				return E_INVALIDARG;
		}
	}

	{
		HWND hWnd = piciex->hwnd;
		if (!hWnd && m_pBrowser)
			m_pBrowser->GetWindow(&hWnd);
		switch (uID)
		{
			case ID_PARENT_NEW_FOLDER:
				DoCreateFolder(hWnd);
				break;
			case ID_PARENT_NEW_SHORTCUT:
				DoCreateShortcut(hWnd);
				break;
			case ID_PARENT_PROPERTY:
				DoProperty(hWnd);
				break;
			case ID_PARENT_PASTE:
				DoPaste(hWnd);
				break;
			default:
				return E_INVALIDARG;
		}
	}
	return S_OK;
}

STDMETHODIMP CFTPFileDirectoryMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax)
{
	idCmd += ID_PARENT_BASE;

	bool bUnicode = false;
	if (uType & GCS_UNICODE)
	{
		bUnicode = true;
		uType &= ~GCS_UNICODE;
	}
	CMyStringW str;
	switch (idCmd)
	{
		case ID_PARENT_NEW_FOLDER:
			str = L"newfolder";
			break;
		case ID_PARENT_NEW_SHORTCUT:
			str = L"newlink";
			break;
		case ID_PARENT_PROPERTY:
			str = L"properties";
			break;
		default:
			return E_NOTIMPL;
	}
	if (uType == GCS_VALIDATEA)
		return S_OK;
	else if (uType == GCS_HELPTEXTA)
	{
		if (!str.LoadString((UINT) idCmd))
			str.Empty();
	}
	UINT dw = (UINT) (bUnicode ? str.GetLength() : str.GetLengthA());
	if (cchMax > dw + 1)
		cchMax = dw + 1;
	if (bUnicode)
	{
		memcpy(pszName, (LPCWSTR) str, sizeof(WCHAR) * (cchMax - 1));
		((LPWSTR) pszName)[cchMax - 1] = 0;
	}
	else
	{
		memcpy(pszName, (LPCSTR) str, sizeof(CHAR) * (cchMax - 1));
		((LPSTR) pszName)[cchMax - 1] = 0;
	}
	return S_OK;
}

STDMETHODIMP CFTPFileDirectoryMenu::SetSite(IUnknown* pUnkSite)
{
	if (m_pUnkSite)
		m_pUnkSite->Release();
	m_pUnkSite = pUnkSite;
	if (pUnkSite)
	{
		pUnkSite->AddRef();

		IShellBrowser* pBrowser;
		HRESULT hr = pUnkSite->QueryInterface(IID_IShellBrowser, (void**) &pBrowser);
		if (SUCCEEDED(hr))
		{
			if (m_pBrowser)
				m_pBrowser->Release();
			m_pBrowser = pBrowser;
		}
	}
	return S_OK;
}

STDMETHODIMP CFTPFileDirectoryMenu::GetSite(REFIID riid, void** ppvSite)
{
	if (!ppvSite)
		return E_POINTER;
	if (!m_pUnkSite)
	{
		*ppvSite = NULL;
		return E_FAIL;
	}
	return m_pUnkSite->QueryInterface(riid, ppvSite);
}

void CFTPFileDirectoryMenu::DoCreateFolder(HWND hWndOwner)
{
	CMyStringW str;
	if (FileNameDialog(str, hWndOwner, true, false))
		m_pParent->m_pRoot->CreateFTPDirectory(hWndOwner, m_pParent, str);
}

void CFTPFileDirectoryMenu::DoCreateShortcut(HWND hWndOwner)
{
	CLinkDialog dlg;
	dlg.m_strCurDir = m_pParent->m_strDirectory;
	if (!m_pParent->m_pRoot->PreShowCreateShortcutDialog(&dlg))
		return;
	if (dlg.ModalDialogW(hWndOwner) == IDOK)
	{
		m_pParent->m_pRoot->CreateShortcut(hWndOwner, m_pParent, dlg.m_strFileName, dlg.m_strLinkTo, dlg.m_bHardLink);
	}
}

void CFTPFileDirectoryMenu::DoProperty(HWND hWndOwner)
{
	IShellFolder* pParent = m_pParent->GetParentFolder();
	if (pParent)
	{
		PITEMID_CHILD pidlC = ::GetChildItemIDList(m_pidlMe);
		IContextMenu* pMenu = NULL;
		PCITEMID_CHILD pcidlC = pidlC;
		HRESULT hr = pParent->GetUIObjectOf(hWndOwner, 1, &pcidlC, IID_IContextMenu, NULL, (void**) &pMenu);
		if (SUCCEEDED(hr) && pMenu)
		{
			CMINVOKECOMMANDINFOEX ciex;
			ciex.cbSize = sizeof(ciex);
			ciex.fMask = CMIC_MASK_UNICODE;
			ciex.lpVerbW = L"properties";
			ciex.hwnd = hWndOwner;
			pMenu->InvokeCommand((CMINVOKECOMMANDINFO*) &ciex);
			pMenu->Release();
		}
		::CoTaskMemFree(pidlC);
	}
}

void CFTPFileDirectoryMenu::DoPaste(HWND hWndOwner)
{
	IDataObject* pObject = NULL;
	auto hr = ::OleGetClipboard(&pObject);
	if (FAILED(hr) || pObject == NULL)
	{
		::MessageBeep(0);
		return;
	}

	FORMATETC fmt;
	STGMEDIUM stg;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.ptd = NULL;
	fmt.tymed = TYMED_HGLOBAL;
	fmt.cfFormat = theApp.m_nCFPreferredDropEffect;
	hr = pObject->QueryGetData(&fmt);
	if (hr == S_OK)
	{
		hr = pObject->GetData(&fmt, &stg);
		if (SUCCEEDED(hr))
		{
			auto pv = ::GlobalLock(stg.hGlobal);
			if (pv)
			{
				DWORD dwEffect = *reinterpret_cast<DWORD*>(pv);
				::GlobalUnlock(stg.hGlobal);
				::ReleaseStgMedium(&stg);

				hr = CFTPDropHandler::PerformDrop(pObject, dwEffect, m_pParent, hWndOwner, &dwEffect);
			}
		}
	}
	pObject->Release();
}
