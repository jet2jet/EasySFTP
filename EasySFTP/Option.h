
#pragma once

class COptionDialog : public CMyDialog
{
public:
	COptionDialog();
	virtual ~COptionDialog();

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual bool OnInitDialog(HWND hWndFocus);
	LRESULT OnClearAllCredentials(WPARAM wParam, LPARAM lParam);
	LRESULT OnRegister(WPARAM wParam, LPARAM lParam);
	LRESULT OnOK(WPARAM wParam, LPARAM lParam);

private:
	IEasySFTPRoot2* m_pRoot;
};
