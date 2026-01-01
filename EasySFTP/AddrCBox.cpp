/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 AddrCBox.cpp - implementations of CAddressComboBox and CVirtualAddressComboBox
 */

#include "StdAfx.h"
#include "EasySFTP.h"
#include "AddrCBox.h"

#include "IDList.h"

////////////////////////////////////////////////////////////////////////////////

struct CAddressComboBoxItemData
{
	PIDLIST_ABSOLUTE lpidl;
	//IShellFolder* pFolder;
	int iLevel;
	bool bCurrent;
	int iIcon;
	//HICON hIcon, hIconSel;
	CMyStringW strDisplayName;
	CAddressComboBoxItemData* pParent;
	CAddressComboBoxItemData* pChild;
	CAddressComboBoxItemData* pNext;

public:
	CAddressComboBoxItemData()
	{
		//hIcon = hIconSel = NULL;
		iIcon = -1;
	}
};

CAddressComboBox::CAddressComboBox(void)
{
	m_bUseDisplayName = false;
	//m_nCurSel = -1;
	::SHGetDesktopFolder(&m_pFolderRoot);
	m_pFolder = NULL;
}

CAddressComboBox::~CAddressComboBox(void)
{
	if (m_pFolder)
		m_pFolder->Release();
	m_pFolderRoot->Release();
}

HWND CAddressComboBox::Create(int x, int y, int cx, int cy, HWND hWndParent, UINT uID)
{
	//return CreateExW(0, L"ComboBox", NULL,
	//	WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
	//	CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | CBS_OWNERDRAWFIXED,
	//	x, y, cx, cy, hWndParent, (HMENU) UIntToPtr(uID));
	//return CreateExW(0, L"Static", NULL,
	//	WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	//	x, y, cx, cy, hWndParent, (HMENU) UIntToPtr(uID));
	return CreateExW(0, WC_COMBOBOXEXW, NULL,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		CBS_DROPDOWN | CBS_AUTOHSCROLL,
		x, y, cx, cy, hWndParent, (HMENU) UIntToPtr(uID));
}

// 既に子が1つある場合、重複追加はしない (2つ以上の場合は考慮していない)
// 戻り値: MyComputerがあればそのデータを返す
static CAddressComboBoxItemData* __stdcall AddChildren(HWND hWnd,
	CAddressComboBoxItemData* pParent, IShellFolder* pFolder)
{
	CAddressComboBoxItemData* pData, * pData2, * pChild, * pRet;
	PITEMID_CHILD lp;
	PIDLIST_ABSOLUTE lp2;
	HRESULT hr;
	IEnumIDList* pEnum;
	SFGAOF rgf;
	int iLevel;
	bool bHasEasySFTPRoot = false;

	pRet = NULL;
	iLevel = pParent->iLevel + 1;
	if (iLevel != 1)
		bHasEasySFTPRoot = true;
	hr = pFolder->EnumObjects(hWnd,
		SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FASTITEMS,
		&pEnum);
	if (hr == S_OK)
	{
		pData2 = NULL;
		pChild = pParent->pChild;
		pParent->pChild = NULL;
		while (true)
		{
			hr = pEnum->Next(1, &lp, NULL);
			if (hr != S_OK)
			{
				if (bHasEasySFTPRoot)
					break;
				lp = (PITEMID_CHILD) ::DuplicateItemIDList(theApp.m_pidlEasySFTP);
				rgf = SFGAO_FILESYSTEM;
				bHasEasySFTPRoot = true;
			}
			else
			{
				rgf = SFGAO_FILESYSTEM;
				hr = pFolder->GetAttributesOf(1, &lp, &rgf);
				if (FAILED(hr))
					break;
			}
			lp2 = AppendItemIDList(pParent->lpidl, lp);
			::CoTaskMemFree(lp);
			//if (!IsMyComputerIDList(lp2) &&
			//	!(rgf & SFGAO_FILESYSTEM))
			//{
			//	::CoTaskMemFree(lp2);
			//	continue;
			//}
			if (!bHasEasySFTPRoot && IsEqualIDList(lp2, theApp.m_pidlEasySFTP))
				bHasEasySFTPRoot = true;
			if (pChild && IsEqualIDList(pChild->lpidl, lp2))
			{
				pData = pChild;
				pChild = NULL;
				::CoTaskMemFree(lp2);
				lp2 = pData->lpidl;
			}
			else
			{
				//if (pData2 == NULL)
				//	level++;
				pData = new CAddressComboBoxItemData();
				pData->pParent = pParent;
				pData->iLevel = iLevel;
				pData->bCurrent = false;
				pData->pChild = NULL;
				pData->lpidl = lp2;
			}
			if (pData2)
				pData2->pNext = pData;
			else
				pParent->pChild = pData;
			pData->pNext = NULL;
			pData2 = pData;
			if (IsMyComputerIDList(lp2))
				pRet = pData;
		}
		pEnum->Release();
		if (pChild)
		{
			if (pData2)
				pData2->pNext = pChild;
			else
				pParent->pChild = pChild;
		}
	}
	return pRet;
}

static void __stdcall AddChildrenToComboBox(HWND hWnd, bool bUnicode, CAddressComboBoxItemData* pData, CAddressComboBoxItemData* pCur)
{
	int i;
	union
	{
		COMBOBOXEXITEMA cbeiA;
		COMBOBOXEXITEMW cbeiW;
	};
	(bUnicode ? cbeiW.mask : cbeiA.mask) = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_INDENT;
	while (pData)
	{
		if (bUnicode)
		{
			cbeiW.iItem = -1;
			cbeiW.iIndent = pData->iLevel;
			cbeiW.iImage = I_IMAGECALLBACK;
			cbeiW.iSelectedImage = I_IMAGECALLBACK;
			cbeiW.pszText = LPSTR_TEXTCALLBACKW;
			cbeiW.lParam = (LPARAM) pData;
			i = (int) ::SendMessage(hWnd, CBEM_INSERTITEMW, 0, (LPARAM) &cbeiW);
		}
		else
		{
			cbeiA.iItem = -1;
			cbeiA.iIndent = pData->iLevel;
			cbeiA.iImage = I_IMAGECALLBACK;
			cbeiA.iSelectedImage = I_IMAGECALLBACK;
			cbeiA.pszText = LPSTR_TEXTCALLBACKA;
			cbeiA.lParam = (LPARAM) pData;
			i = (int) ::SendMessage(hWnd, CBEM_INSERTITEMA, 0, (LPARAM) &cbeiA);
		}
		if (pData == pCur)
			::SendMessage(hWnd, CB_SETCURSEL, (WPARAM) IntToPtr(i), 0);
		if (pData->pChild)
			AddChildrenToComboBox(hWnd, bUnicode, pData->pChild, pCur);
		pData = pData->pNext;
	}
}

static void __stdcall DeleteChildrenFromComboBox(HWND hWnd, int nPos, CAddressComboBoxItemData* pData)
{
	if (pData->pChild)
	{
		CAddressComboBoxItemData* p = pData->pChild;
		int nPos2 = nPos + 1;
		while (p)
		{
			CAddressComboBoxItemData* p2 = p;
			p = p->pNext;
			DeleteChildrenFromComboBox(hWnd, nPos2, p2);
		}
	}
	::SendMessage(hWnd, CBEM_DELETEITEM, (WPARAM) nPos, 0);
}

void CAddressComboBox::ChangeCurrentFolder(PCIDLIST_ABSOLUTE lpidl)
{
	int nCount = (int) (::SendMessage(m_hWnd, CB_GETCOUNT, 0, 0));

	CAddressComboBoxItemData* pData, * pData2, * pChild, * pTop, * pCur;
	PIDLIST_ABSOLUTE lp;
	//LPITEMIDLIST lp2;
	IShellFolder* pFolder;
	int level;
	HRESULT hr;

	lp = (PIDLIST_ABSOLUTE) DuplicateItemIDList(lpidl);
	pData2 = NULL;
	pChild = NULL;
	level = 0;
	while (lp)
	{
		pData = new CAddressComboBoxItemData();
		if (pData2)
		{
			pData2->pParent = pData;
			pData->pChild = pData2;
		}
		else
			pData->pChild = NULL;
		if (pData->bCurrent = !pChild)
			pChild = pData;
		pData->lpidl = lp;
		pData->pParent = NULL;
		pData->pNext = NULL;
		pData2 = pData;
		//level++;
		lp = RemoveOneChild(lp);
	}
	pCur = pChild;
	pData = pTop = pData2;
	level = 0;
	while (pData)
	{
		pData->iLevel = level++;
		pData = pData->pChild;
	}

	//if (IsDesktopIDList(lpidl))
	//{
	//	pFolder = pDesktop;
	//	pDesktop->AddRef();
	//}
	//else
	//{
	//	hr = pDesktop->BindToObject(lpidl, NULL, IID_IShellFolder, (void**) &pFolder);
	//}
	//if (SUCCEEDED(hr))
	//{
	//	IEnumIDList* pEnum;
	//	SFGAOF rgf;

	//	hr = pFolder->EnumObjects(m_hWnd,
	//		SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN | SHCONTF_FASTITEMS,
	//		&pEnum);
	//	if (hr == S_OK)
	//	{
	//		pData2 = NULL;
	//		while (true)
	//		{
	//			hr = pEnum->Next(1, &lp, NULL);
	//			if (hr != S_OK)
	//				break;
	//			rgf = SFGAO_FILESYSTEM;
	//			hr = pFolder->GetAttributesOf(1, (LPCITEMIDLIST*) &lp, &rgf);
	//			if (FAILED(hr))
	//				break;
	//			lp2 = AppendItemIDList(lpidl, lp);
	//			::CoTaskMemFree(lp);
	//			if (!IsMyComputerIDList(lp2) &&
	//				!(rgf & SFGAO_FILESYSTEM))
	//			{
	//				::CoTaskMemFree(lp2);
	//				continue;
	//			}
	//			//if (pData2 == NULL)
	//			//	level++;
	//			pData = new CAddressComboBoxItemData();
	//			if (pData2)
	//			{
	//				pData2->pNext = pData;
	//				pData->pParent = pData2->pParent;
	//				pData->iLevel = pData2->iLevel;
	//			}
	//			else
	//			{
	//				pData->pParent = pChild;
	//				pData->iLevel = pChild->iLevel + 1;
	//				pChild->pChild = pData;
	//				pChild = pData;
	//			}
	//			pData->bCurrent = false;
	//			pData->pChild = NULL;
	//			pData->lpidl = lp2;
	//			pData->pNext = NULL;
	//			pData2 = pData;
	//		}
	//		pEnum->Release();
	//	}
	//	pFolder->Release();
	//}

	pData = AddChildren(m_hWnd, pTop, m_pFolderRoot);
	if (pData)
	{
		hr = m_pFolderRoot->BindToObject(pData->lpidl, NULL, IID_IShellFolder, (void**) &pFolder);
		if (SUCCEEDED(hr))
		{
			AddChildren(m_hWnd, pData, pFolder);
			pFolder->Release();
		}
	}

	//cur = --level;
	//pData2 = pChild;
	//while (pData2)
	//{
	//	::SendMessage(m_pWndComboBox->m_hWnd, CB_INSERTSTRING, 0, (LPARAM) pData2);
	//	pData2->iLevel = level;
	//	if (pData2->pNext)
	//		pData2 = pData2->pNext;
	//	else
	//		pData2 = pData2->pParent;
	//}
	AddChildrenToComboBox(m_hWnd, m_bUnicode, pTop, pCur);
	while (nCount--)
		::SendMessage(m_hWnd, CBEM_DELETEITEM, (WPARAM) IntToPtr(nCount), 0);

	//((CAddressComboBoxMain*) m_pWndComboBox)->m_nCurSel = PtrToInt(::SendMessage(m_pWndComboBox->m_hWnd, CB_GETCURSEL, 0, 0));
	m_nCurSel = (int) (::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0));
	{
		if (IsDesktopIDList(lpidl))
		{
			FillData(pTop);
			m_strRealPath = pTop->strDisplayName;
		}
		else
		{
			IShellFolder* pParent;
			lp = RemoveOneChild(lpidl);
			if (IsDesktopIDList(lp))
			{
				pParent = m_pFolderRoot;
				pParent->AddRef();
				hr = S_OK;
			}
			else
				hr = m_pFolderRoot->BindToObject(lp, NULL, IID_IShellFolder, (void**) &pParent);
			::CoTaskMemFree(lp);
			if (SUCCEEDED(hr))
			{
				STRRET strret;
				strret.pOleStr = NULL;
				strret.uType = STRRET_WSTR;
				lp = (PIDLIST_ABSOLUTE) GetChildItemIDList(lpidl);
				hr = pParent->GetDisplayNameOf((PCITEMID_CHILD) lp,
					SHGDN_NORMAL | SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, &strret);
				if (SUCCEEDED(hr))
				{
					switch (strret.uType)
					{
						case STRRET_WSTR:
							m_strRealPath = strret.pOleStr;
							::CoTaskMemFree(strret.pOleStr);
							break;
						case STRRET_CSTR:
							m_strRealPath = strret.cStr;
							break;
						case STRRET_OFFSET:
							m_strRealPath = (LPCSTR) (((LPCBYTE) lp) + strret.uOffset);
							break;
					}
				}
				else if (!::SHGetPathFromIDListW(lpidl, m_strRealPath.GetBuffer(MAX_PATH)))
				{
					::SHGetPathFromIDListA(lpidl, m_strRealPath.GetBufferA(MAX_PATH));
					m_strRealPath.ReleaseBufferA();
				}
				else
					m_strRealPath.ReleaseBuffer();
				::CoTaskMemFree(lp);
				pParent->Release();
			}
		}
	}
	UpdateRealPath(m_strRealPath);
}

void CAddressComboBox::NotifyChange(WPARAM wParam, LPARAM lParam)
{
	int nCode = (int) lParam;
	if (!(nCode & (SHCNE_MKDIR | SHCNE_RENAMEFOLDER | SHCNE_RMDIR | SHCNE_DRIVEADD | SHCNE_DRIVEADDGUI | SHCNE_DRIVEREMOVED)))
		return;
	const PCIDLIST_ABSOLUTE* ppidlItem = (const PCIDLIST_ABSOLUTE*) wParam;
	PCIDLIST_ABSOLUTE pidlBase = ppidlItem[0];
	if (!pidlBase->mkid.cb)
		return;
	bool bTargetIsSingleIDList = ::IsSingleIDList(pidlBase);

	PIDLIST_ABSOLUTE lpidlComputer;
	HRESULT hr;

	hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &lpidlComputer);
	if (FAILED(hr))
		return;

	IShellFolder* pDesktop;
	hr = ::SHGetDesktopFolder(&pDesktop);
	if (FAILED(hr))
	{
		::CoTaskMemFree(lpidlComputer);
		return;
	}
	CAddressComboBoxItemData* pDesktopData = NULL;
	CAddressComboBoxItemData* pData = NULL;
	CAddressComboBoxItemData* pMyComputer = NULL;
	int nCount = (int) ::SendMessage(m_hWnd, CB_GETCOUNT, 0, 0);
	int nItem = CB_ERR;
	int nItemMyComputer = CB_ERR;
	for (int i = 0; i < nCount; i++)
	{
		CAddressComboBoxItemData* pDataTemp = (CAddressComboBoxItemData*) GetItemData(i);
		if (bTargetIsSingleIDList)
		{
			if (pDataTemp->iLevel == 1)
			{
				hr = pDesktop->CompareIDs(0, pDataTemp->lpidl, pidlBase);
				int iRet = (short) HRESULT_CODE(hr);
				if (iRet == 0)
				{
					pData = pDataTemp;
					nItem = i;
				}
			}
		}
		else if (IsEqualIDList(pDataTemp->lpidl, pidlBase))
		{
			pData = pDataTemp;
			nItem = i;
			//break;
		}
		if (IsEqualIDList(pDataTemp->lpidl, lpidlComputer))
		{
			pMyComputer = pDataTemp;
			nItemMyComputer = i;
		}
		if (i == 0)
			pDesktopData = pDataTemp;
	}
	if (nItem == CB_ERR)
	{
		if (!bTargetIsSingleIDList)
		{
			// Check whether the item is in My Computer
			if (!IsMatchParentIDList(lpidlComputer, pidlBase))
			{
				::CoTaskMemFree(lpidlComputer);
				return;
			}
			// Retrieve drive's idlist
			PCUIDLIST_RELATIVE pidlDrive = (PCUIDLIST_RELATIVE)(((const BYTE UNALIGNED*) pidlBase) + pidlBase->mkid.cb);
			// If the target refers to the children, then we ignore it
			if (((PCUIDLIST_RELATIVE)(((const BYTE UNALIGNED*) pidlDrive) + pidlDrive->mkid.cb))->mkid.cb)
			{
				::CoTaskMemFree(lpidlComputer);
				return;
			}
		}
	}
	switch (nCode)
	{
		case SHCNE_MKDIR:
			pData = new CAddressComboBoxItemData();
			pData->lpidl = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(pidlBase);
			pData->iLevel = 1;
			pData->iIcon = -1;
			pData->bCurrent = false;
			pData->pChild = pData->pNext = NULL;
			pData->pParent = pDesktopData;
			if (!pDesktopData->pChild)
				pDesktopData->pChild = pData;
			else
			{
				pMyComputer = pDesktopData->pChild;
				while (pMyComputer->pNext)
					pMyComputer = pMyComputer->pNext;
				pMyComputer->pNext = pData;
			}
			AddChildrenToComboBox(m_hWnd, m_bUnicode, pData, NULL);
			break;
		case SHCNE_RENAMEFOLDER:
			if (nItem != CB_ERR)
			{
				::CoTaskMemFree(pData->lpidl);
				pData->lpidl = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(ppidlItem[1]);
				pData->iIcon = -1;
				union
				{
					COMBOBOXEXITEMA cbeiA;
					COMBOBOXEXITEMW cbeiW;
				};
				(m_bUnicode ? cbeiW.mask : cbeiA.mask) = CBEIF_TEXT;
				if (m_bUnicode)
				{
					cbeiW.iItem = nItem;
					cbeiW.pszText = LPSTR_TEXTCALLBACKW;
					::SendMessage(m_hWnd, CBEM_SETITEMW, 0, (LPARAM) &cbeiW);
				}
				else
				{
					cbeiA.iItem = nItem;
					cbeiA.pszText = LPSTR_TEXTCALLBACKA;
					::SendMessage(m_hWnd, CBEM_SETITEMA, 0, (LPARAM) &cbeiA);
				}
			}
			break;
		case SHCNE_RMDIR:
			if (nItem != CB_ERR)
			{
				pMyComputer = pData->pParent->pChild;
				if (pMyComputer == pData)
					pData->pParent->pChild = pData->pNext;
				else
				{
					while (pMyComputer && pMyComputer->pNext != pData)
						pMyComputer = pMyComputer->pNext;
					if (pMyComputer)
						pMyComputer->pNext = pData->pNext;
				}
				DeleteChildrenFromComboBox(m_hWnd, nItem, pData);
			}
			break;
		case SHCNE_DRIVEADD:
		case SHCNE_DRIVEADDGUI:
		{
			pData = new CAddressComboBoxItemData();
			pData->lpidl = (PIDLIST_ABSOLUTE) ::DuplicateItemIDList(pidlBase);
			pData->iLevel = pMyComputer->iLevel + 1;
			pData->iIcon = -1;
			pData->bCurrent = false;
			pData->pChild = pData->pNext = NULL;
			pData->pParent = pDesktopData;
			int iPos;
			iPos = nItemMyComputer + 1;
			if (!pMyComputer->pChild)
				pMyComputer->pChild = pData;
			else
			{
				IShellFolder* pFolder;
				hr = pDesktop->BindToObject(pMyComputer->lpidl, NULL, IID_IShellFolder, (void**) &pFolder);
				if (FAILED(hr))
					pFolder = NULL;
				pDesktopData = pMyComputer->pChild;
				if (pFolder)
				{
					PCUIDLIST_RELATIVE pidl1 = pidlBase;
					PCUIDLIST_RELATIVE pidl2 = pDesktopData->lpidl;
					while (!IsSingleIDList(pidl1))
						pidl1 = (PCUIDLIST_RELATIVE)(((const BYTE UNALIGNED*) pidl1) + pidl1->mkid.cb);
					while (!IsSingleIDList(pidl2))
						pidl2 = (PCUIDLIST_RELATIVE)(((const BYTE UNALIGNED*) pidl2) + pidl2->mkid.cb);
					hr = pFolder->CompareIDs(0, pidl1, pidl2);
					int iRet = (short) HRESULT_CODE(hr);
					if (iRet < 0)
					{
						pData->pNext = pDesktopData;
						pMyComputer->pChild = pData;
					}
					else
					{
						while (pDesktopData->pNext)
						{
							iPos++;
							pidl2 = pDesktopData->pNext->lpidl;
							while (!IsSingleIDList(pidl2))
								pidl2 = (PCUIDLIST_RELATIVE)(((const BYTE UNALIGNED*) pidl2) + pidl2->mkid.cb);
							hr = pFolder->CompareIDs(0, pidl1, pidl2);
							iRet = (short) HRESULT_CODE(hr);
							if (iRet < 0)
								break;
							pDesktopData = pDesktopData->pNext;
						}
						pData->pNext = pDesktopData->pNext;
						pDesktopData->pNext = pData;
					}
					pFolder->Release();
				}
				else
				{
					while (pDesktopData->pNext)
					{
						pDesktopData = pDesktopData->pNext;
						iPos++;
					}
					pDesktopData->pNext = pData;
				}
			}

			union
			{
				COMBOBOXEXITEMA cbeiA;
				COMBOBOXEXITEMW cbeiW;
			};
			(m_bUnicode ? cbeiW.mask : cbeiA.mask) = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_INDENT;
			if (m_bUnicode)
			{
				cbeiW.iItem = iPos;
				cbeiW.iIndent = pData->iLevel;
				cbeiW.iImage = I_IMAGECALLBACK;
				cbeiW.iSelectedImage = I_IMAGECALLBACK;
				cbeiW.pszText = LPSTR_TEXTCALLBACKW;
				cbeiW.lParam = (LPARAM) pData;
				::SendMessage(m_hWnd, CBEM_INSERTITEMW, 0, (LPARAM) &cbeiW);
			}
			else
			{
				cbeiA.iItem = iPos;
				cbeiA.iIndent = pData->iLevel;
				cbeiA.iImage = I_IMAGECALLBACK;
				cbeiA.iSelectedImage = I_IMAGECALLBACK;
				cbeiA.pszText = LPSTR_TEXTCALLBACKA;
				cbeiA.lParam = (LPARAM) pData;
				::SendMessage(m_hWnd, CBEM_INSERTITEMA, 0, (LPARAM) &cbeiA);
			}
		}
		break;
		case SHCNE_DRIVEREMOVED:
			if (nItem != CB_ERR)
			{
				pDesktopData = pMyComputer->pChild;
				if (pDesktopData == pData)
					pMyComputer->pChild = pData->pNext;
				else
				{
					while (pDesktopData && pDesktopData->pNext != pData)
						pDesktopData = pDesktopData->pNext;
					if (pDesktopData)
						pDesktopData->pNext = pData->pNext;
				}
				DeleteChildrenFromComboBox(m_hWnd, nItem, pData);
			}
			break;
	}
	pDesktop->Release();
	::CoTaskMemFree(lpidlComputer);
}

PCIDLIST_ABSOLUTE CAddressComboBox::GetSelectedFolder() const
{
	int i;
	i = (int) ::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0);
	if (i == CB_ERR)
		return NULL;
	CAddressComboBoxItemData* pData = (CAddressComboBoxItemData*) GetItemData(i);
	return pData ? pData->lpidl : NULL;
}

PCIDLIST_ABSOLUTE CAddressComboBox::FindItemFromDisplayName(LPCWSTR lpszDisplayName) const
{
	int nCount;
	nCount = (int) (::SendMessage(m_hWnd, CB_GETCOUNT, 0, 0));
	for (int i = 0; i < nCount; i++)
	{
		CAddressComboBoxItemData* pData = (CAddressComboBoxItemData*) GetItemData(i);
		FillData(pData);
		if (pData && pData->strDisplayName.Compare(lpszDisplayName) == 0)
			return pData->lpidl;
	}
	return NULL;
}

void* CAddressComboBox::GetItemData(int nIndex) const
{
	CAddressComboBoxItemData* pData;
	if (m_bUnicode)
	{
		COMBOBOXEXITEMW cei;
		cei.mask = CBEIF_LPARAM;
		cei.iItem = nIndex;
		cei.lParam = 0;
		::SendMessage(m_hWnd, CBEM_GETITEMW, 0, (LPARAM) &cei);
		pData = (CAddressComboBoxItemData*) cei.lParam;
	}
	else
	{
		COMBOBOXEXITEMA cei;
		cei.mask = CBEIF_LPARAM;
		cei.iItem = nIndex;
		cei.lParam = 0;
		::SendMessage(m_hWnd, CBEM_GETITEMA, 0, (LPARAM) &cei);
		pData = (CAddressComboBoxItemData*) cei.lParam;
	}
	return pData;
}

bool CAddressComboBox::SetImageList()
{
	HRESULT hr;
	PIDLIST_ABSOLUTE pidlDesktop;
	SHFILEINFO sfi;

	hr = ::SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidlDesktop);
	if (FAILED(hr))
		return false;
	memset(&sfi, 0, sizeof(sfi));
	m_himlSystemSmall = (HIMAGELIST) ::SHGetFileInfo((LPCTSTR)pidlDesktop, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	::CoTaskMemFree(pidlDesktop);
	if (!m_himlSystemSmall)
		return false;

	::SendMessage(m_hWnd, CBEM_SETIMAGELIST, 0, (LPARAM) m_himlSystemSmall);
	//::SendMessage(m_hWnd, CB_SETEXTENDEDUI, (WPARAM) TRUE, 0);
	return true;
}

void CAddressComboBox::UpdateRealPath(LPCWSTR lpszRealPath)
{
	//((CAddressComboBoxMain*) m_pWndComboBox)->m_wndEdit.SetWindowTextW(lpszRealPath);
	CMyStringW str(lpszRealPath);
	CAddressComboBoxItemData* pData = (CAddressComboBoxItemData*) GetItemData(m_nCurSel);

	union
	{
		COMBOBOXEXITEMA cbeiA;
		COMBOBOXEXITEMW cbeiW;
	};
	(m_bUnicode ? cbeiW.mask : cbeiA.mask) = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_INDENT;
	if (m_bUnicode)
	{
		cbeiW.iItem = -1;
		cbeiW.iIndent = pData->iLevel;
		cbeiW.iImage = I_IMAGECALLBACK;
		cbeiW.iSelectedImage = I_IMAGECALLBACK;
		cbeiW.pszText = (LPWSTR)(LPCWSTR) str;
		cbeiW.lParam = (LPARAM) pData;
		::SendMessage(m_hWnd, CBEM_SETITEMW, 0, (LPARAM) &cbeiW);
	}
	else
	{
		cbeiA.iItem = -1;
		cbeiA.iIndent = pData->iLevel;
		cbeiA.iImage = I_IMAGECALLBACK;
		cbeiA.iSelectedImage = I_IMAGECALLBACK;
		cbeiA.pszText = (LPSTR)(LPCSTR) str;
		cbeiA.lParam = (LPARAM) pData;
		::SendMessage(m_hWnd, CBEM_SETITEMA, 0, (LPARAM) &cbeiA);
	}
	//SetWindowTextW(lpszRealPath);
}

void CAddressComboBox::RestoreTextBox()
{
	//::SendMessage(m_hWnd, CB_SETCURSEL, (WPARAM) IntToPtr(m_nCurSel), 0);
	//((CAddressComboBoxMain*) m_pWndComboBox)->m_wndEdit.SetWindowTextW(m_strRealPath);
	SetWindowTextW(m_strRealPath);
	//int i = (int) ::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0);
	//CAddressComboBoxItemData* pData = (CAddressComboBoxItemData*) GetItemData(m_nCurSel);
	//FillData(pData);
	//if (pData)
	//	SetWindowTextW(pData->strDisplayName);
}

//void CAddressComboBox::RestoreToSelItem()
//{
//	CAddressComboBoxItemData* pData;
//	if (m_bUnicode)
//	{
//		COMBOBOXEXITEMW cei;
//		cei.mask = CBEIF_LPARAM;
//		cei.iItem = m_nCurSel;
//		cei.lParam = 0;
//		::SendMessage(m_hWnd, CBEM_GETITEMW, 0, (LPARAM) &cei);
//		pData = (CAddressComboBoxItemData*) cei.lParam;
//	}
//	else
//	{
//		COMBOBOXEXITEMA cei;
//		cei.mask = CBEIF_LPARAM;
//		cei.iItem = m_nCurSel;
//		cei.lParam = 0;
//		::SendMessage(m_hWnd, CBEM_GETITEMA, 0, (LPARAM) &cei);
//		pData = (CAddressComboBoxItemData*) cei.lParam;
//	}
//	if (pData)
//		SetWindowTextW(pData->strDisplayName);
//	::SendMessage(m_hWnd, CB_SETCURSEL, (WPARAM) IntToPtr(m_nCurSel), 0);
//}

LRESULT CAddressComboBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE_PROC_MESSAGE(WM_CREATE, OnCreate);
	HANDLE_PROC_MESSAGE(WM_KEYDOWN, OnKeyDown);
	HANDLE_REFLECT_NOTIFY(CBEN_DELETEITEM, OnDeleteItem);
	HANDLE_REFLECT_NOTIFY(CBEN_GETDISPINFOA, OnGetDispInfoA);
	HANDLE_REFLECT_NOTIFY(CBEN_GETDISPINFOW, OnGetDispInfoW);
	HANDLE_REFLECT_NOTIFY(CBEN_BEGINEDIT, OnBeginEdit);

	return CMyWindow::WindowProc(message, wParam, lParam);
}

LRESULT CAddressComboBox::OnCreate(WPARAM wParam, LPARAM lParam)
{
	if (Default(wParam, lParam) != 0)
		return 1;

	::SendMessage(m_hWnd, CBEM_SETUNICODEFORMAT, (WPARAM) TRUE, 0);
	m_bUnicode =
		(::SendMessage(m_hWnd, CBEM_GETUNICODEFORMAT, 0, 0) != 0);

	if (!SetImageList())
		return 1;

	return 0;
}

LRESULT CAddressComboBox::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	if ((UINT) wParam == VK_DOWN)
		return 0;
	return Default(wParam, lParam);
}

void CAddressComboBox::FillData(void* _pData) const
{
	if (!_pData)
		return;
	CAddressComboBoxItemData* pData = (CAddressComboBoxItemData*) _pData;

	if (pData->iIcon == -1)
	{
		UINT uFlags;
		SHFILEINFO_UNION sfi;

		uFlags = SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_DISPLAYNAME | SHGFI_PIDL;
		if (pData->bCurrent)
			uFlags |= SHGFI_OPENICON;
		sfi.w.szDisplayName[0] = 0;
		if (!::SHGetFileInfoW((LPCWSTR) pData->lpidl, 0, &sfi.w, sizeof(sfi.w), uFlags))
		{
			sfi.a.szDisplayName[0] = 0;
			::SHGetFileInfoA((LPCSTR) pData->lpidl, 0, &sfi.a, sizeof(sfi.a), uFlags);
			pData->strDisplayName = sfi.a.szDisplayName;
			pData->iIcon = sfi.a.iIcon;
			//uFlags |= SHGFI_SELECTED;
			//uFlags &= ~SHGFI_DISPLAYNAME;
			//::SHGetFileInfoA((LPCSTR) pData->lpidl, 0, &sfi.a, sizeof(sfi.a), uFlags);
			//pData->iIconSel = sfi.a.iIcon;
		}
		else
		{
			pData->iIcon = sfi.w.iIcon;
			pData->strDisplayName = sfi.w.szDisplayName;
			//uFlags |= SHGFI_SELECTED;
			//uFlags &= ~SHGFI_DISPLAYNAME;
			//::SHGetFileInfoW((LPCWSTR) pData->lpidl, 0, &sfi.w, sizeof(sfi.w), uFlags);
			//pData->iIconSel = sfi.w.iIcon;
		}
	}
}

void CAddressComboBox::DeleteData(void* _pData) const
{
	CAddressComboBoxItemData* pData = (CAddressComboBoxItemData*) _pData;
	//if (pData->pFolder)
	//	pData->pFolder->Release();
	//::DestroyIcon(pData->hIcon);
	//::DestroyIcon(pData->hIconSel);
	if (pData->lpidl)
		::CoTaskMemFree(pData->lpidl);
	delete pData;
}

LRESULT CAddressComboBox::OnDeleteItem(WPARAM wParam, LPARAM lParam)
{
	// we assume lParam as NMCOMBOBOXEXW (must be same as the size of NMCOMBOBOXEXA and NMCOMBOBOXEXW)
	PNMCOMBOBOXEXW lpnm = (PNMCOMBOBOXEXW) lParam;
	CAddressComboBoxItemData* pData = (CAddressComboBoxItemData*) lpnm->ceItem.lParam;
	lpnm->ceItem.lParam = 0;
	if (pData)
		DeleteData(pData);
	return 0;
}

LRESULT CAddressComboBox::OnGetDispInfoA(WPARAM wParam, LPARAM lParam)
{
	PNMCOMBOBOXEXA lpnm = (PNMCOMBOBOXEXA) lParam;
	CAddressComboBoxItemData* pData;
	if (lpnm->ceItem.iItem == -1 && !lpnm->ceItem.lParam)
		pData = (CAddressComboBoxItemData*) GetItemData(m_nCurSel);
	else
		pData = (CAddressComboBoxItemData*) lpnm->ceItem.lParam;
	if (pData)
	{
		FillData(pData);
		if (lpnm->ceItem.mask & CBEIF_IMAGE)
			lpnm->ceItem.iImage = pData->iIcon;
		if (lpnm->ceItem.mask & CBEIF_SELECTEDIMAGE)
			lpnm->ceItem.iSelectedImage = pData->iIcon;
		if (lpnm->ceItem.mask & CBEIF_TEXT)
		{
			// NOTE: Windows may contain the buffer to retreive text information
			// (and may cause buffer corruption when we return the string pointer)
			if (lpnm->ceItem.pszText && lpnm->ceItem.cchTextMax)
			{
				size_t nLen = (size_t) pData->strDisplayName.GetLengthA() + 1;
				if (nLen > (size_t) lpnm->ceItem.cchTextMax)
					nLen = lpnm->ceItem.cchTextMax;
				strncpy_s(lpnm->ceItem.pszText, (size_t) lpnm->ceItem.cchTextMax, pData->strDisplayName, nLen);
			}
			else
				lpnm->ceItem.pszText = (LPSTR)(LPCSTR) pData->strDisplayName;
		}
	}
	lpnm->ceItem.mask |= CBEIF_DI_SETITEM;
	return 0;
}

LRESULT CAddressComboBox::OnGetDispInfoW(WPARAM wParam, LPARAM lParam)
{
	PNMCOMBOBOXEXW lpnm = (PNMCOMBOBOXEXW) lParam;
	CAddressComboBoxItemData* pData;
	if (lpnm->ceItem.iItem == -1 && !lpnm->ceItem.lParam)
		pData = (CAddressComboBoxItemData*) GetItemData(m_nCurSel);
	else
		pData = (CAddressComboBoxItemData*) lpnm->ceItem.lParam;
	if (pData)
	{
		FillData(pData);
		if (lpnm->ceItem.mask & CBEIF_IMAGE)
			lpnm->ceItem.iImage = pData->iIcon;
		if (lpnm->ceItem.mask & CBEIF_SELECTEDIMAGE)
			lpnm->ceItem.iSelectedImage = pData->iIcon;
		if (lpnm->ceItem.mask & CBEIF_TEXT)
		{
			// NOTE: Windows may contain the buffer to retreive text information
			// (and may cause buffer corruption when we return the string pointer)
			if (lpnm->ceItem.pszText && lpnm->ceItem.cchTextMax)
			{
				size_t nLen = (size_t) pData->strDisplayName.GetLength() + 1;
				if (nLen > (size_t) lpnm->ceItem.cchTextMax)
					nLen = lpnm->ceItem.cchTextMax;
				wcsncpy_s(lpnm->ceItem.pszText, (size_t) lpnm->ceItem.cchTextMax, pData->strDisplayName, nLen);
			}
			else
				lpnm->ceItem.pszText = (LPWSTR)(LPCWSTR) pData->strDisplayName;
		}
	}
	lpnm->ceItem.mask |= CBEIF_DI_SETITEM;
	return 0;
}

LRESULT CAddressComboBox::OnBeginEdit(WPARAM wParam, LPARAM lParam)
{
	int i = (int) ::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0);
	if (i != CB_ERR)
		m_nCurSel = i;
	UpdateRealPath(m_strRealPath);
	return 0;
}

bool CAddressComboBox::HandleEndEdit(int iWhy, bool bChanged, LPCWSTR lpszText, HWND hWndNextFocus)
{
	switch (iWhy)
	{
		//case CBENF_DROPDOWN:
		//{
		//	if (!bChanged)
		//	{
		//		int i = (int) ::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0);
		//		if (i != CB_ERR)
		//			m_nCurSel = i;
		//		CAddressComboBoxItemData* pData = (CAddressComboBoxItemData*) GetItemData(m_nCurSel);
		//		FillData(pData);
		//		if (pData)
		//			SetWindowTextW(pData->strDisplayName);
		//		//if (i == CB_ERR)
		//		//	::SendMessage(m_hWnd, CB_SETCURSEL, (WPARAM) m_nCurSel, 0);
		//	}
		//}
		//break;
		case CBENF_RETURN:
		{
			if (bChanged)
				return true;
			//else
			//{
			//	RestoreTextBox();
			//	::SetFocus(hWndNextFocus);
			//}
		}
		break;
		//case CBENF_KILLFOCUS:
		//	if (!bChanged)
		//		RestoreTextBox();
		//	break;
		case CBENF_ESCAPE:
			RestoreTextBox();
			//::SetFocus(hWndNextFocus);
			break;
	}
	return false;
}
