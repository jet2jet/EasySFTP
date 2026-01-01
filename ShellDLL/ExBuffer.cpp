/*
 Copyright (C) 2010 jet (ジェット)

 ExBuffer.cpp - implementations of CExBuffer
 */

#include "StdAfx.h"
#include "ExBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BUFFER_BLOCK_SIZE     1024

CExBuffer::CExBuffer(size_t nInitSize)
{
	nInitSize = BUFFER_BLOCK_SIZE * ((size_t) ((nInitSize + BUFFER_BLOCK_SIZE - 1) / BUFFER_BLOCK_SIZE));
	m_pvBuffer = malloc(nInitSize);
	m_nPos = 0;
	m_nMemSize = nInitSize;
}

CExBuffer::~CExBuffer()
{
	if (m_pvBuffer)
		free(m_pvBuffer);
}

CExBuffer& CExBuffer::operator = (const CExBuffer& buffer)
{
	ExpandBuffer(buffer.m_nLen);
	memcpy(m_pvBuffer, buffer.m_pvBuffer, buffer.m_nLen);
	m_nPos = 0;
	m_nLen = buffer.m_nLen;
	return *this;
}

bool CExBuffer::ExpandBuffer(size_t nReqSize)
{
	void* pv;
	size_t nSize;
	nSize = ROUNDUP(nReqSize, (size_t) BUFFER_BLOCK_SIZE);
	if (nSize <= m_nMemSize)
		return true;
	pv = realloc(m_pvBuffer, nSize);
	if (!pv)
		return false;
	m_pvBuffer = pv;
	m_nMemSize = nSize;
	return true;
}

void* CExBuffer::AppendToBuffer(const void* pvData, size_t nSize)
{
	DeleteForwardData();
	//if (m_nPos + nSize > m_nMemSize)
	if (m_nLen + nSize > m_nMemSize)
	{
		//if (!ExpandBuffer(m_nPos + nSize))
		if (!ExpandBuffer(m_nLen + nSize))
			return NULL;
	}
	BYTE* p = (BYTE*) m_pvBuffer + m_nLen;
	if (pvData)
		memcpy(p, pvData, nSize);
	m_nLen += nSize;
	return p;
}

bool CExBuffer::GetAndSkip(LPSTR& psz, size_t nLen)
{
	void* pv = GetCurrentBufferPermanentAndSkip(nLen);
	if (!pv)
		return false;
	psz = (LPSTR) malloc(nLen + 1);
	if (!psz)
		return false;
	memcpy(psz, pv, nLen);
	psz[nLen] = 0;
	return true;
}

void* CExBuffer::GetCurrentBuffer(size_t nSize)
{
	if (m_nPos + nSize > m_nMemSize)
	{
		if (!ExpandBuffer(m_nPos + nSize))
			return NULL;
	}
	if (m_nPos + nSize > m_nLen)
		m_nLen = m_nPos + nSize;
	return (BYTE*) m_pvBuffer + m_nPos;
}

void* CExBuffer::GetCurrentBufferAndSkip(size_t nSize)
{
	if (m_nPos + nSize > m_nMemSize)
	{
		if (!ExpandBuffer(m_nPos + nSize))
			return NULL;
	}
	void* pvRet = (BYTE*) m_pvBuffer + m_nPos;
	m_nPos += nSize;
	if (m_nPos > m_nLen)
		m_nLen = m_nPos;
	return pvRet;
}

void* CExBuffer::GetCurrentBufferPermanent(size_t nSize)
{
	if (m_nPos + nSize > m_nLen)
		return NULL;
	return (BYTE*) m_pvBuffer + m_nPos;
}

void* CExBuffer::GetCurrentBufferPermanentAndSkip(size_t nSize)
{
	if (m_nPos + nSize > m_nLen)
		return NULL;
	void* pvRet = (BYTE*) m_pvBuffer + m_nPos;
	m_nPos += nSize;
	return pvRet;
}

void CExBuffer::DeleteForwardData()
{
	if (m_pvBuffer && m_nPos && m_nPos < m_nLen)
		memmove(m_pvBuffer, (BYTE*) m_pvBuffer + m_nPos, m_nLen - m_nPos);
	m_nLen -= m_nPos;
	m_nPos = 0;
}

void CExBuffer::DeleteLastData(size_t nSizeLast)
{
	if (m_nLen - m_nPos < nSizeLast)
		m_nLen = m_nPos;
	else
		m_nLen -= nSizeLast;
}

void CExBuffer::Empty()
{
	m_nLen = 0;
	m_nPos = 0;
}

bool CExBuffer::SetDataToBuffer(const void* pvData, size_t nSize)
{
	DeleteForwardData();
	if (nSize > m_nMemSize)
	{
		if (!ExpandBuffer(nSize))
			return false;
	}
	memcpy(m_pvBuffer, pvData, nSize);
	m_nLen = nSize;
	//m_nPos = 0;
	return true;
}
