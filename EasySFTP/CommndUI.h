/*
 EasySFTP - Copyright (C) 2010 jet (ジェット)

 CommndUI.h - declarations of CCommandUIItem and inheritances
 */

#pragma once

class __declspec(novtable) CCommandUIItem
{
public:
	virtual bool Enable(bool bOnOff) = 0;
	virtual bool Check(bool bOnOff) = 0;
	virtual bool ChangeBitmap(int iImage) = 0;
	virtual UINT GetID() = 0;
};

class CMenuItem : public CCommandUIItem
{
public:
	CMenuItem(HMENU hMenu)
	{
		m_hMenu = hMenu;
	}

	HMENU m_hMenu;
	UINT m_uID;

	virtual bool Enable(bool bOnOff)
	{
		::EnableMenuItem(m_hMenu, m_uID, (bOnOff ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
		return true;
	}

	virtual bool Check(bool bCheck)
	{
		::CheckMenuItem(m_hMenu, m_uID, (bCheck ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		return true;
	}

	virtual bool ChangeBitmap(int iImage) { return true; }

	virtual UINT GetID()
		{ return m_uID; }
};

class CToolBarItem : public CCommandUIItem
{
public:
	CToolBarItem(HWND hWndToolBar)
	{
		m_hWndToolBar = hWndToolBar;
	}

	HWND m_hWndToolBar;
	UINT m_uID;

	virtual bool Enable(bool bOnOff)
	{
		return ::SendMessage(m_hWndToolBar, TB_ENABLEBUTTON, (WPARAM) m_uID, MAKELPARAM(bOnOff ? TRUE : FALSE, 0)) != 0;
	}

	virtual bool Check(bool bCheck)
	{
		if (!::SendMessage(m_hWndToolBar, TB_ISBUTTONENABLED, (WPARAM) m_uID, 0))
		{
			if (!::SendMessage(m_hWndToolBar, TB_ISBUTTONCHECKED, (WPARAM) m_uID, 0))
				return true;
			bCheck = false;
		}
		return ::SendMessage(m_hWndToolBar, TB_CHECKBUTTON, (WPARAM) m_uID, MAKELPARAM(bCheck ? TRUE : FALSE, 0)) != 0;
	}

	virtual bool ChangeBitmap(int iImage)
	{
		if ((int) (::SendMessage(m_hWndToolBar, TB_GETBITMAP, (WPARAM) m_uID, 0)) == iImage)
			return true;
		return ::SendMessage(m_hWndToolBar, TB_CHANGEBITMAP, (WPARAM) m_uID, MAKELPARAM(iImage, 0)) != 0;
	}

	virtual UINT GetID()
		{ return m_uID; }
};
