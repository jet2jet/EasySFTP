#pragma once

#include "ExBuffer.h"
#include "Unknown.h"

enum class SSHChannelReturnType
{
	Success = 1,
	Again = 0,
	Error = -1
};

class CSSHChannel;

class __declspec(novtable) CSSHChannelListener //: public IUnknown
{
public:
	virtual void ChannelOpenFailure(CSSHChannel* pChannel, int nReason) = 0;
	virtual void ChannelError(CSSHChannel* pChannel, int nReason) = 0;
	virtual void ChannelOpened(CSSHChannel* pChannel) = 0;
	virtual void ChannelClosed(CSSHChannel* pChannel) = 0;
	virtual void ChannelExitStatus(CSSHChannel* pChannel, int nExitCode) = 0;
	// response for SendProcessStartup
	virtual void ChannelConfirm(CSSHChannel* pChannel, bool bSucceeded, int nReason) = 0;
	virtual void ChannelDataReceived(CSSHChannel* pChannel, const void* pvData, size_t nSize) = 0;
};

class CSSHChannel : public CUnknownImpl
{
public:
	CSSHChannel(CSSHChannelListener* pListener);
	virtual ~CSSHChannel();

	SSHChannelReturnType InitializeChannel(LIBSSH2_SESSION* pSession);
	bool SendProcessStartup(LPCSTR lpszService, const void* pvExtraData, size_t nExtraDataLen);
	bool SendChannelData(const void* pvSendData, size_t nSendDataLen);
	bool CheckChannelWindow(DWORD dwLength);
	void CloseChannel();

	bool IsSendingData() const;
	// returns false if the process is not necessary on next (Win32) message loop
	bool Process(bool isSocketReceived);

protected:
	virtual void ProcessChannelData(CExBuffer& data);

private:
	bool ProcessPendingStartup();
	bool ProcessSendChannelData();
	bool ProcessRead();
	bool ProcessWindowAdjust();

protected:
	CSSHChannelListener* m_pListener;

private:
	LIBSSH2_CHANNEL* m_pChannel;
	struct
	{
		LPCSTR lpszService;
		const void* pvExtraData;
		size_t nExtraDataLen;
		bool isPending;
	} m_pendingStartup;
	struct
	{
		CExBuffer sendData;
		CExBuffer pendingData;
		bool isPending;
		bool isSending;
	} m_pendingSendChannelData;
	struct
	{
		DWORD dwReadLen;
		void* pvBuffer;
	} m_pendingRead;
	struct
	{
		DWORD dwLength;
		bool isPending;
	} m_pendingWindowAdjust;
	CExBuffer m_pendingBuffer;
};
