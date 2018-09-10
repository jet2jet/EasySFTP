/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 ESFTPFld.h - declarations of EasySFTP interfaces
 */

#pragma once

#include "TextMode.h"

#define TEXTMODE_LOCAL_CRLF   TEXTMODE_BUFFER_CRLF
#define TEXTMODE_LOCAL_CR     TEXTMODE_BUFFER_CR
#define TEXTMODE_LOCAL_LF     TEXTMODE_BUFFER_LF
#define TEXTMODE_LOCAL_MASK   TEXTMODE_BUFFER_MASK
#define TEXTMODE_SERVER_CRLF  TEXTMODE_STREAM_CRLF
#define TEXTMODE_SERVER_CR    TEXTMODE_STREAM_CR
#define TEXTMODE_SERVER_LF    TEXTMODE_STREAM_LF
#define TEXTMODE_SERVER_MASK  TEXTMODE_STREAM_MASK

// Returns the text convertion mode when writing data to the server.
// If this value is non-zero, we call MyCreateTextStream, with its parameters
// set to this value and IStream for write.
//#define TEXTMODE_FOR_SEND(tm) TEXTMODE_CHANGE_BUFFER_AND_STREAM_NO_ENCODE(tm)
#define TEXTMODE_FOR_SEND(tm) (tm & TEXTMODE_RETURN_MASK)
// Returns the text convertion mode when receiving data from the server.
// If this value is non-zero, we call MyCreateTextStream, with its parameters
// set to this value and IStream for read.
#define TEXTMODE_FOR_RECV(tm) (tm & TEXTMODE_RETURN_MASK)
// Returns the text convertion mode when writing data to the server.
// Note that this macro is used when the local data is supplied as an instance of IStream.
// If this value is non-zero, we call MyCreateTextStream, with its parameters
// set to this value and IStream of local data for read.
//#define TEXTMODE_FOR_SEND_LOCAL_STREAM(tm) (tm & TEXTMODE_RETURN_MASK)
#define TEXTMODE_FOR_SEND_LOCAL_STREAM(tm) TEXTMODE_CHANGE_BUFFER_AND_STREAM_NO_ENCODE(tm)
// Returns the text convertion mode when receiving data from the server.
// Note that this macro is used when the local data is supplied as an instance of IStream.
// If this value is non-zero, we call MyCreateTextStream, with its parameters
// set to this value and IStream of local data for write.
#define TEXTMODE_FOR_RECV_LOCAL_STREAM(tm) TEXTMODE_CHANGE_BUFFER_AND_STREAM_NO_ENCODE(tm)

#define TRANSFER_MODE_AUTO    0
#define TRANSFER_MODE_TEXT    1
#define TRANSFER_MODE_BINARY  2

#undef INTERFACE
#define INTERFACE IEasySFTPListener

DECLARE_INTERFACE_IID_(IEasySFTPListener, IUnknown, "AD29C042-B9E3-463a-9DF6-D7DA5B8D0199")
{
    // *** IUnknown methods ***
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	// *** IEasySFTPListener methods ***
	STDMETHOD(ChangeLocalDirectory)(THIS_ LPCWSTR lpszPath) PURE;
};

EXTERN_C const IID IID_IEasySFTPListener;

#undef INTERFACE
#define INTERFACE IEasySFTPDirectory

DECLARE_INTERFACE_IID_(IEasySFTPDirectory, IUnknown, "AD29C042-B9E3-463e-9DF6-D7DA5B8D0199")
{
    // *** IUnknown methods ***
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	// *** IEasySFTPDirectory methods ***
	STDMETHOD(GetRootDirectory)(THIS_ IEasySFTPDirectory FAR* FAR* ppRootDirectory) PURE;

	STDMETHOD(GetHostInfo)(THIS_ VARIANT_BOOL FAR* pbIsSFTP, int FAR* pnPort, BSTR FAR* pbstrHostName) PURE;
	STDMETHOD(GetTextMode)(THIS_ LONG FAR* pnTextMode) PURE;
	STDMETHOD(SetTextMode)(THIS_ LONG nTextMode) PURE;
	STDMETHOD(GetTransferMode)(THIS_ LONG FAR* pnTransferMode) PURE;
	STDMETHOD(SetTransferMode)(THIS_ LONG nTransferMode) PURE;
	STDMETHOD(IsTextFile)(THIS_ LPCWSTR lpszFileName) PURE;
	STDMETHOD(Disconnect)(THIS) PURE;
	STDMETHOD(IsConnected)(THIS) PURE;
	STDMETHOD(IsTransferring)(THIS) PURE;
};

EXTERN_C const IID IID_IEasySFTPDirectory;

#undef INTERFACE
#define INTERFACE IEasySFTPRoot

DECLARE_INTERFACE_IID_(IEasySFTPRoot, IUnknown, "AD29C042-B9E3-463c-9DF6-D7DA5B8D0199")
{
    // *** IUnknown methods ***
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	// *** IEasySFTPRoot methods ***
	//STDMETHOD(SetListener)(THIS_ IEasySFTPListener* pListener) PURE;
	STDMETHOD(Connect)(THIS_ VARIANT_BOOL bSFTP, HWND hWnd, const void* pvReserved,
		LPCWSTR lpszHostName, int nPort, IShellFolder FAR* FAR* ppFolder) PURE;
	STDMETHOD(QuickConnectDialog)(THIS_ HWND hWndOwner, IShellFolder FAR* FAR* ppFolder) PURE;
};

EXTERN_C const IID IID_IEasySFTPRoot;
EXTERN_C const CLSID CLSID_EasySFTP;

STDAPI EasySFTPCreateRoot(IEasySFTPRoot** ppRoot);
