#include "stdafx.h"
#include "ShellDLL.h"
#include "Pageant.h"

#define AGENT_MAX_MSGLEN  262144

#define GET_32BIT_MSB_FIRST(cp) \
  (((unsigned long)(unsigned char)(cp)[0] << 24) | \
  ((unsigned long)(unsigned char)(cp)[1] << 16) | \
  ((unsigned long)(unsigned char)(cp)[2] << 8) | \
  ((unsigned long)(unsigned char)(cp)[3]))

// source code from https://git.tartarus.org/?p=simon/putty.git;a=blob;f=windows/utils/security.c;h=2d899b834dcb0f5d2d9942e55939c1b4f729da25

static PSID usersid;

PSID get_user_sid(void)
{
	HANDLE proc = NULL, tok = NULL;
	TOKEN_USER* user = NULL;
	DWORD toklen, sidlen;
	PSID sid = NULL, ret = NULL;

	if (usersid)
		return usersid;

	if ((proc = OpenProcess(MAXIMUM_ALLOWED, false,
		GetCurrentProcessId())) == NULL)
		goto cleanup;

	if (!OpenProcessToken(proc, TOKEN_QUERY, &tok))
		goto cleanup;

	if (!GetTokenInformation(tok, TokenUser, NULL, 0, &toklen) &&
		GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		goto cleanup;

	if ((user = (TOKEN_USER*)LocalAlloc(LPTR, toklen)) == NULL)
		goto cleanup;

	if (!GetTokenInformation(tok, TokenUser, user, toklen, &toklen))
		goto cleanup;

	sidlen = GetLengthSid(user->User.Sid);

	sid = (PSID) malloc(sidlen);

	if (!CopySid(sidlen, sid, user->User.Sid))
		goto cleanup;

	/* Success. Move sid into the return value slot, and null it out
	 * to stop the cleanup code freeing it. */
	ret = usersid = sid;
	sid = NULL;

cleanup:
	if (proc != NULL)
		CloseHandle(proc);
	if (tok != NULL)
		CloseHandle(tok);
	if (user != NULL)
		LocalFree(user);
	if (sid != NULL)
		free(sid);

	return ret;
}

// source code come from PuTTY: https://git.tartarus.org/?p=simon/putty.git;a=blob;f=windows/agent-client.c;h=4eb0bcfb9667ecb6f0f63c1fb8d5ea70651d04ba

#define AGENT_COPYDATA_ID 0x804e50ba   /* random goop */

bool CPageantAgent::IsAvailable()
{
	HWND hwnd;
	hwnd = FindWindow(_T("Pageant"), _T("Pageant"));
	if (!hwnd)
		return false;
	else
		return true;
}

bool CPageantAgent::Query(const void* dataSend, size_t dataSendSize, void** dataReceived, size_t* dataReceivedSize)
{
	HWND hwnd;
	char mapname[23];
	HANDLE filemap;
	unsigned char* p, * ret;
	int id, retlen;
	COPYDATASTRUCT cds;
	SECURITY_ATTRIBUTES sa, * psa;
	PSECURITY_DESCRIPTOR psd = NULL;
	PSID usersid = NULL;

	*dataReceived = NULL;
	*dataReceivedSize = 0;

	if (dataSendSize > AGENT_MAX_MSGLEN)
		return false;          /* query too large */

	hwnd = FindWindow(_T("Pageant"), _T("Pageant"));
	if (!hwnd)
		return false;          /* *dataReceived == NULL, so failure */
	_snprintf_s(mapname, sizeof(mapname), sizeof(mapname), "PageantRequest%08x", (unsigned)GetCurrentThreadId());

	psa = NULL;

	/*
	 * Make the file mapping we create for communication with
	 * Pageant owned by the user SID rather than the default. This
	 * should make communication between processes with slightly
	 * different contexts more reliable: in particular, command
	 * prompts launched as administrator should still be able to
	 * run PSFTPs which refer back to the owning user's
	 * unprivileged Pageant.
	 */
	usersid = get_user_sid();

	if (usersid) {
		psd = (PSECURITY_DESCRIPTOR)
			LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
		if (psd)
		{
			if (InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION) &&
				SetSecurityDescriptorOwner(psd, usersid, false)) {
				sa.nLength = sizeof(sa);
				sa.bInheritHandle = true;
				sa.lpSecurityDescriptor = psd;
				psa = &sa;
			}
			else
			{
				LocalFree(psd);
				psd = NULL;
			}
		}
	}

	filemap = CreateFileMapping(INVALID_HANDLE_VALUE, psa, PAGE_READWRITE,
		0, AGENT_MAX_MSGLEN, mapname);
	if (filemap == NULL || filemap == INVALID_HANDLE_VALUE)
	{
		if (psd)
			LocalFree(psd);
		return false;	       /* *dataReceived == NULL, so failure */
	}
	p = (unsigned char*) MapViewOfFile(filemap, FILE_MAP_WRITE, 0, 0, 0);
	memcpy(p, dataSend, dataSendSize);
	cds.dwData = AGENT_COPYDATA_ID;
	cds.cbData = 1 + strlen(mapname);
	cds.lpData = mapname;

	/*
	 * The user either passed a null callback (indicating that the
	 * query is required to be synchronous) or CreateThread failed.
	 * Either way, we need a synchronous request.
	 */
	id = SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
	if (id > 0) {
		uint32_t length_field = GET_32BIT_MSB_FIRST(p);
		if (length_field > 0 && length_field <= AGENT_MAX_MSGLEN - 4)
		{
			retlen = 4 + GET_32BIT_MSB_FIRST(p);
			ret = (unsigned char*) malloc(sizeof(unsigned char) * retlen);
			if (ret) {
				memcpy(ret, p, retlen);
				*dataReceived = ret;
				*dataReceivedSize = retlen;
			}
		}
		else
		{
			/*
			 * If we get here, we received an out-of-range length
			 * field, either without space for a message type code or
			 * overflowing the FileMapping.
			 *
			 * Treat this as if Pageant didn't answer at all - which
			 * actually means we do nothing, and just don't fill in
			 * out and outlen.
			 */
		}
	}
	UnmapViewOfFile(p);
	CloseHandle(filemap);
	if (psd)
		LocalFree(psd);
	return true;
}
