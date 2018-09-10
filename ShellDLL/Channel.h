/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 Channel.h - declarations of CSSH2Channel
 */

#pragma once

// channel types
#define SSH_CHANNEL_X11_LISTENER    1	/* Listening for inet X11 conn. */
#define SSH_CHANNEL_PORT_LISTENER   2	/* Listening on a port. */
#define SSH_CHANNEL_OPENING         3	/* waiting for confirmation */
#define SSH_CHANNEL_OPEN            4	/* normal open two-way channel */
#define SSH_CHANNEL_CLOSED          5	/* waiting for close confirmation */
#define SSH_CHANNEL_AUTH_SOCKET     6	/* authentication socket */
#define SSH_CHANNEL_X11_OPEN        7	/* reading first X11 packet */
#define SSH_CHANNEL_INPUT_DRAINING  8	/* sending remaining data to conn */
#define SSH_CHANNEL_OUTPUT_DRAINING 9	/* sending remaining data to app */
#define SSH_CHANNEL_LARVAL          10	/* larval session */
#define SSH_CHANNEL_RPORT_LISTENER  11	/* Listening to a R-style port  */
#define SSH_CHANNEL_CONNECTING      12
#define SSH_CHANNEL_DYNAMIC         13
#define SSH_CHANNEL_ZOMBIE          14	/* Almost dead. */
#define SSH_CHANNEL_MAX_TYPE        15

#define CHAN_SES_PACKET_DEFAULT    (32 * 1024)
#define CHAN_SES_WINDOW_DEFAULT    (64 * CHAN_SES_PACKET_DEFAULT)

enum channel_type
{
	TYPE_SHELL, TYPE_PORTFWD, TYPE_SCP, TYPE_SFTP, TYPE_AGENT,
};

class CSSH2Channel;

#include "UString.h"
#include "ExBuffer.h"

#include "Unknown.h"

class __declspec(novtable) CSSH2ChannelListener //: public IUnknown
{
public:
	virtual DWORD ChannelGetServerCompatible() = 0;
	virtual bool ChannelSendPacket(BYTE bType, const void* pData, size_t nSize) = 0;
	virtual void ChannelOpenFailure(CSSH2Channel* pChannel, int nReason, const CMyStringW& strMessage) = 0;
	virtual void ChannelOpened(CSSH2Channel* pChannel) = 0;
	virtual void ChannelClosed(CSSH2Channel* pChannel) = 0;
	virtual void ChannelExitStatus(CSSH2Channel* pChannel, int nExitCode) = 0;
	virtual void ChannelConfirm(CSSH2Channel* pChannel, bool bSucceeded) = 0;
	virtual void ChannelDataReceived(CSSH2Channel* pChannel, const void* pvData, size_t nSize) = 0;
};

class CSSH2Channel : public CUnknownImpl
{
public:
	CSSH2Channel(CSSH2ChannelListener* pListener, LPCSTR lpszType, int nStatus, LPCSTR lpszRemoteName);
	CSSH2Channel(CSSH2ChannelListener* pListener, LPCSTR lpszType, int nStatus, LPCSTR lpszRemoteName, UINT nLocalWindow, UINT nLocalMaxPacket);
	virtual ~CSSH2Channel();

	bool OpenChannel();
	bool SendChannelRequest(LPCSTR lpszService, bool bWantConfirm, const void* pvExtraData, size_t nExtraDataLen);
	bool SendChannelData(const void* pvSendData, size_t nSendDataLen);
	bool CheckChannelWindow();
	void CloseChannel();
	size_t AvailableRemoteWindowSize() const
		{ return (size_t) m_nRemoteWindow; }

	static bool __stdcall ProcessChannelMsg(CSSH2ChannelListener* pListener, BYTE bType, const void* pv, size_t nSize);

public:
	UINT m_nID;
	UINT m_nRemoteID;
	LPCSTR m_lpszType;
	int m_nStatus;
	LPCSTR m_lpszRemoteName;
	UINT m_nLocalWindow, m_nLocalWindowMax;
	UINT m_nLocalReceived;
	UINT m_nLocalMaxPacket;
	UINT m_nRemoteWindow;
	UINT m_nRemoteMaxPacket;
	bool m_bClosed;
	CExBuffer m_bufferKeepChannelData;

protected:
	CSSH2ChannelListener* m_pListener;
	DWORD m_dwServerCompat;

private:
	void CommonConstruct();

protected:
	virtual bool ProcessChannelClose(CExBuffer& buffer);
	virtual bool ProcessChannelData(CExBuffer& buffer);
	virtual bool ProcessChannelEOF(CExBuffer& buffer);
	virtual bool ProcessChannelExtendedData(CExBuffer& buffer);
	virtual bool ProcessChannelOpenConfirmation(CExBuffer& buffer);
	virtual bool ProcessChannelOpenFailure(CExBuffer& buffer);
	virtual bool ProcessChannelRequest(CExBuffer& buffer);
	virtual bool ProcessChannelWindowAdjust(CExBuffer& buffer);
	virtual bool ProcessChannelSuccess(CExBuffer& buffer);
	virtual bool ProcessChannelFailure(CExBuffer& buffer);
};
