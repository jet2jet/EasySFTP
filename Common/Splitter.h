/*
 Copyright (C) 2010 jet (ジェット)

 Splitter.h - declarations of functions and definitions for Splitter window
 */

#ifndef __SPLITTER_H__
#define __SPLITTER_H__

#ifndef WM_NOTIFY
#define WM_NOTIFY              0x004E

typedef struct tagNMHDR
{
	HWND hwndFrom;
	UINT idFrom;
	UINT code;
} NMHDR, * PNMHDR, FAR* LPNMHDR;
#endif // WM_NOTIFY

#define SPLITTER_CLASSW        L"SplitterBar"
#define SPLITTER_CLASSA        "SplitterBar"

#ifdef UNICODE
#define SPLITTER_CLASS         SPLITTER_CLASSW
#else
#define SPLITTER_CLASS         SPLITTER_CLASSA
#endif

#define SPS_HORIZONTAL         0x0000
#define SPS_VERTICAL           0x0001

// return 1 to prevent splitter from tracking
#define SPN_BEFORETRACK        1
// return 1 to prevent splitter from moving
#define SPN_TRACK              2
// return 1 to change splitter's position
#define SPN_TRACKING           3
// return value is the brush object (can be NULL)
#define SPN_GETBRUSH           4
// return 1 to prevent splitter from tracking
#define SPN_DBLCLK             5

// no parameter; no return value
#define SPM_BEGINTRACK         (WM_USER + 1)
// no parameter; return value is the width of the splitter bar
#define SPM_GETWIDTH           (WM_USER + 2)
// wParam: low-order word is the new width of the splitter bar;
// return value is the old width
#define SPM_SETWIDTH           (WM_USER + 3)
// no parameter; return value is the handle of the cursor
#define SPM_GETCURSOR          (WM_USER + 4)
// wParam: Handle to the new cursor (cannot be NULL); no return value
#define SPM_SETCURSOR          (WM_USER + 5)

typedef struct _SPTRACKNOTIFY
{
	NMHDR hdr;
	int nPos;
	RECT rcWindow;
	RECT rcParent;
} SPTRACKNOTIFY, * PSPTRACKNOTIFY, NEAR* NPSPTRACKNOTIFY, FAR* LPSPTRACKNOTIFY;

extern "C" bool __stdcall InitSplitter(HINSTANCE hInstance);

#endif // __SPLITTER_H__
