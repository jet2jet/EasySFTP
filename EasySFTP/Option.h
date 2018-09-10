
#pragma once

class COptionDialog : public CMyDialog
{
public:
	COptionDialog();

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual bool OnInitDialog(HWND hWndFocus);
	LRESULT OnRegister(WPARAM wParam, LPARAM lParam);
	LRESULT OnOK(WPARAM wParam, LPARAM lParam);
};
