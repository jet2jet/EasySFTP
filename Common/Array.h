// Array.h v2.16 (2011/12/11) Copyright (C) 2011 Kuri-Applcations
//  v2.16
//    ---CMySimpleArrayT---
//     èCê≥
//      (ñ≥å¯Ç»ÉCÉìÉfÉbÉNÉXÇ…Ç®ÇØÇÈBreakñΩóﬂÇí«â¡)
//     í«â¡
//      GrowCount, RemoveAllCompletely
//  v2.15
//    ---CMySimpleArrayT---
//     èCê≥
//      _QuickSortInner
//    ---Global---
//     í«â¡
//      DoQuickSortArray, DoQuickSortArrayEx
//  v2.14
//    ---CMySimpleArrayT---
//     èCê≥
//      CompareItem
//
//     í«â¡
//      Sort
//  v2.13
//    ---CMySimpleArrayT---
//     èCê≥
//      CopyArray, MoveArray, SetArray (for memory leak problem)
//
//     í«â¡
//      CMyPtrArray, CMyPtrArrayT<T>, CMyPtrArrayPtrT<PTR_T> (for reducing code size for templates)
//  v2.12
//    ---CMySimpleArrayT---
//     èCê≥
//      AddArray
//    ---CMyStringArrayA/W---
//     í«â¡
//      CMyStringArrayA/W(const CMyStringArrayA/W&), CMyStringArrayA/W(ptr, count)
//  v2.11
//    ---CMySimpleArrayT---
//     í«â¡
//      CMySimpleArrayT(const CMySimpleArrayT<T, ARG_T>&), Enqueue, Dequeue,
//        GetFirstStackPosition, GetNextStackPosition,
//        GetFirstQueuePosition, GetNextQueuePosition
//     èCê≥
//      Add, InsertItem
//    ---CMyStringArray---
//     èCê≥
//      CMyStringArray -> CMyStringArrayA, CMyStringArrayW
//  v2.10
//    ---CMySimpleArrayT---
//     í«â¡
//      CMySimpleArrayT(ptr, count)
//      Push, Pop
//  v2.09.2
//     èCê≥
//      operator delete (void*, void*)
//    ---CMySimpleArrayT---
//     èCê≥
//      InsertItem, InsertArray
//  v2.09
//    ---CMySimpleArrayT---
//     í«â¡
//      SortItems
//  v2.08
//    ---CMySimpleArrayT---
//     èCê≥
//      CopyArray, AddArray
//  v2.07
//    ---CMySimpleArrayT---
//     í«â¡
//      InsertArray GetIndexFromPtr
//      m_dwParam
//    ---CMyStringArray---
//     í«â¡
//      SetCaseSensitive
//  v2.06
//    ---CMySimpleArrayT---
//     èCê≥
//      SetItem
//  v2.05
//    ---CMySimpleArrayT---
//     èCê≥
//      Add(item), NewItem, DeleteItem
//  v2.04
//    ---CMySimpleArrayT---
//     í«â¡
//      Add(void), AddArray, GetItemPtr, InsertItem(int), (protected) NewItem
//     èCê≥
//      SetCount

#ifndef __ARRAY_H__
#define __ARRAY_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef new
#ifdef _MSC_VER
#pragma push_macro("new")
#define NEW_MACRO_PUSHED_WITH_MSC
#endif
#undef new
#endif

#ifdef _DEBUG
#if defined(_INC_CRTDBG)
#define ArrayDebugBreak() _CrtDbgBreak()
#elif defined(_INC_WINDOWS)
#define ArrayDebugBreak() DebugBreak();
#else
#define ArrayDebugBreak() ((void) 0)
#endif
#else
#define ArrayDebugBreak() ((void) 0)
#endif

//inline void* operator new (size_t, void* pv) { return pv; }
//#ifdef _MSC_VER
//inline void operator delete (void*, void*) { }
//#endif

template <class T, class ARG_T> class CMySimpleArrayT;

template <class T, class ARG_T> void __stdcall DoQuickSortArray(CMySimpleArrayT<T, ARG_T>& arrData);
template <class T, class ARG_T> void __stdcall DoQuickSortArrayEx(CMySimpleArrayT<T, ARG_T>& arrData,
	int (__stdcall* pfnCompareItem)(const T*, const T*, DWORD));

template <class T, class ARG_T> class CMySimpleArrayT
{
public:
	CMySimpleArrayT();
	CMySimpleArrayT(const CMySimpleArrayT<T, ARG_T>& arr);
	CMySimpleArrayT(const T* arr, int count);
	virtual ~CMySimpleArrayT();

public:
	int Add(ARG_T item);
	int Add();
	BOOL InsertItem(int nIndex, ARG_T item);
	BOOL InsertItem(int nIndex);
	T GetItem(int nIndex) const;
	T& GetItemRef(int nIndex);
	T* GetItemPtr(int nIndex) const;
	void RemoveItem(int nIndex, bool bNoDel = false);
	void RemoveAll();
	void RemoveAllCompletely();
	int GetCount() const;
	BOOL SetItem(int nIndex, ARG_T item);
	int FindItem(ARG_T item) const;
	int GetIndexFromPtr(T* ptr) const;
	void ChangePlace(int nIndex1, int nIndex2);
	void SlideItem(int nIndex, int nNewPlace);
	bool SortItems(const int* pNewIndexArray);
	void MoveArray(CMySimpleArrayT<T, ARG_T>& rArray);
	void CopyArray(const CMySimpleArrayT<T, ARG_T>& rArray);
	int AddArray(const CMySimpleArrayT<T, ARG_T>& rArray);
	int InsertArray(int nIndex, const CMySimpleArrayT<T, ARG_T>& rArray);
	void SetArray(T* pArray, int nCount);
	T operator [] (int nIndex) const;
	T& operator [] (int nIndex);
	void SetCount(int nCount);
	void GrowArray(int nCount);

	void Sort();

	int Push(ARG_T item);
	bool Pop(T* pRet);
	void* GetFirstStackPosition() const;
	bool GetNextStackPosition(void*& pos, T* pRet, int* pnIndex = NULL) const;
	int Enqueue(ARG_T item);
	bool Dequeue(T* pRet);
	void* GetFirstQueuePosition() const;
	bool GetNextQueuePosition(void*& pos, T* pRet, int* pnIndex = NULL) const;

protected:
	T* m_pArray;
	T* m_pTemp;
	int m_nCount;
	int m_nMemCount;
	DWORD m_dwParam;

	static void __stdcall MoveItem(T* p1, const T* p2, DWORD dwParam, int nCount = 1, bool bNewItem = false);
	static int __stdcall CompareItem(const T* p1, const T* p2, DWORD dwParam);
	// Add(void), InsertItem(int), SetCount ÇégÇ§Ç∆Ç´Ç…ÇÃÇ›égóp
	// (ïKóvÇ…âûÇ∂Çƒ operator new (size_t, void*) ÇåƒÇ—èoÇ∑ÉCÉìÉvÉäÉÅÉìÉgÇ)
	static void __stdcall NewItem(T* p, DWORD dwParam, int nCount = 1);
	static void __stdcall DeleteItem(T* p, DWORD dwParam, int nCount = 1);
	static void __stdcall CloneToItem(T* pDest, const T* pSrc, DWORD dwParam);
	static T* __stdcall AllocMemory(size_t nSize, DWORD dwParam);
	static T* __stdcall ReAllocMemory(T* pv, size_t nSize, DWORD dwParam);
	static void __stdcall FreeMemory(T* pv, DWORD dwParam);

	static void __stdcall _QuickSortInner(T* pArray, int left, int right, T* pTemp, DWORD dwParam,
		int (__stdcall* pfnCompareItem)(const T*, const T*, DWORD));
	static void __stdcall QuickSort(T* pArray, int nCount, T* pTemp, DWORD dwParam);

	friend void __stdcall DoQuickSortArray<T, ARG_T>(CMySimpleArrayT<T, ARG_T>& arrData);
	friend void __stdcall DoQuickSortArrayEx<T, ARG_T>(CMySimpleArrayT<T, ARG_T>& arrData,
		int (__stdcall* pfnCompareItem)(const T*, const T*, DWORD));

#ifdef _DEBUG
	static void __stdcall _DumpArray(T* pArray, int nCount, DWORD dwParam);
#endif
};

template <class T, class ARG_T> CMySimpleArrayT<T, ARG_T>::CMySimpleArrayT()
{
	m_dwParam = 0;
	m_pArray = NULL;
	m_nCount = m_nMemCount = 0;
	m_pTemp = AllocMemory(sizeof(T), m_dwParam);
#ifdef _DEBUG
	memset(m_pTemp, 0xFF, sizeof(T));
#endif
}

template <class T, class ARG_T> CMySimpleArrayT<T, ARG_T>::CMySimpleArrayT(const CMySimpleArrayT<T, ARG_T>& arr)
{
	m_pTemp = AllocMemory(sizeof(T), m_dwParam);
#ifdef _DEBUG
	memset(m_pTemp, 0xFF, sizeof(T));
#endif

	m_nCount = arr.m_nCount;
	m_nMemCount = arr.m_nCount;
	m_dwParam = arr.m_dwParam;
	if (m_nCount)
	{
		m_pArray = AllocMemory(sizeof(T) * m_nCount, m_dwParam);
		NewItem(m_pArray, m_dwParam, m_nCount);
		MoveItem(m_pArray, arr.m_pArray, m_dwParam, m_nCount, true);
	}
	else
		m_pArray = NULL;
}

template <class T, class ARG_T> CMySimpleArrayT<T, ARG_T>::CMySimpleArrayT(const T* arr, int nCount)
{
	m_dwParam = 0;
	m_pArray = AllocMemory(sizeof(T) * nCount, m_dwParam);
	for (int i = 0; i < nCount; i++)
		CloneToItem(&m_pArray[i], &arr[i], m_dwParam);
	m_nCount = m_nMemCount = nCount;
	m_pTemp = AllocMemory(sizeof(T), m_dwParam);
#ifdef _DEBUG
	memset(m_pTemp, 0xFF, sizeof(T));
#endif
}

template <class T, class ARG_T> CMySimpleArrayT<T, ARG_T>::~CMySimpleArrayT()
{
	RemoveAll();
	FreeMemory(m_pTemp, m_dwParam);
	FreeMemory(m_pArray, m_dwParam);
}

template <class T, class ARG_T> T* __stdcall CMySimpleArrayT<T, ARG_T>::AllocMemory(size_t nSize, DWORD dwParam)
{
	return (T*) malloc(nSize);
}

template <class T, class ARG_T> T* __stdcall CMySimpleArrayT<T, ARG_T>::ReAllocMemory(T* pv, size_t nSize, DWORD dwParam)
{
	return (T*) realloc(pv, nSize);
}

template <class T, class ARG_T> void __stdcall CMySimpleArrayT<T, ARG_T>::FreeMemory(T* pv, DWORD dwParam)
{
	if (pv)
		free(pv);
}

template <class T, class ARG_T> void __stdcall CMySimpleArrayT<T, ARG_T>::MoveItem(T* p1, const T* p2, DWORD dwParam, int nCount, bool bNewItem)
{
	if (p1 <= p2 || p1 >= (p2 + nCount)) 
	{
		while (nCount--)
		{
			if (bNewItem)
				CloneToItem(p1++, p2++, dwParam);
			else
				*p1++ = *p2++;
		}
	}
	else
	{
		p1 = p1 + nCount - 1;
		p2 = p2 + nCount - 1;

		while (nCount--)
		{
			if (bNewItem)
				CloneToItem(p1--, p2--, dwParam);
			else
				*p1-- = *p2--;
		}
	}
}

template <class T, class ARG_T> int __stdcall CMySimpleArrayT<T, ARG_T>::CompareItem(const T* p1, const T* p2, DWORD dwParam)
{
	return memcmp(p1, p2, sizeof(T));
}

template <class T, class ARG_T> void __stdcall CMySimpleArrayT<T, ARG_T>::NewItem(T* p, DWORD dwParam, int nCount)
{
	while (nCount--)
		new ((void*) p++) T;
}

template <class T, class ARG_T> void __stdcall CMySimpleArrayT<T, ARG_T>::DeleteItem(T* p, DWORD dwParam, int nCount)
{
	while (nCount--)
		(p++)->~T();
}

template <class T, class ARG_T> void __stdcall CMySimpleArrayT<T, ARG_T>::CloneToItem(T* pDest, const T* pSrc, DWORD dwParam)
{
	*pDest = *pSrc;
}

template <class T, class ARG_T> int CMySimpleArrayT<T, ARG_T>::Add(ARG_T item)
{
	if (m_pArray == NULL)
		m_pArray = AllocMemory(sizeof(T) * (m_nMemCount = (m_nCount + 1)), m_dwParam);
	else if (m_nCount >= m_nMemCount)
		m_pArray = ReAllocMemory(m_pArray, (m_nMemCount = (m_nCount + 1)) * sizeof(T), m_dwParam);
	NewItem(&m_pArray[m_nCount], m_dwParam, 1);
	MoveItem(&m_pArray[m_nCount++], (T*) &item, m_dwParam, 1, true);
	return m_nCount - 1;
}

template <class T, class ARG_T> int CMySimpleArrayT<T, ARG_T>::Add()
{
	if (m_pArray == NULL)
		m_pArray = AllocMemory(sizeof(T) * (m_nMemCount = (m_nCount + 1)), m_dwParam);
	else if (m_nCount >= m_nMemCount)
		m_pArray = ReAllocMemory(m_pArray, (m_nMemCount = (m_nCount + 1)) * sizeof(T), m_dwParam);
	// éwíËÉfÅ[É^Ç™ñ≥Ç¢èÍçáÇÕèâä˙âªÇÃÇ›
	NewItem(&m_pArray[m_nCount++], m_dwParam, 1);
	return m_nCount - 1;
}

template <class T, class ARG_T> BOOL CMySimpleArrayT<T, ARG_T>::InsertItem(int nIndex, ARG_T item)
{
	if (nIndex < 0 || nIndex > m_nCount)
	{
		ArrayDebugBreak();
		return FALSE;
	}

	if (m_pArray == NULL)
		m_pArray = AllocMemory(sizeof(T) * (m_nMemCount = (m_nCount + 1)), m_dwParam);
	else if (m_nCount >= m_nMemCount)
		m_pArray = ReAllocMemory(m_pArray, (m_nMemCount = (m_nCount + 1)) * sizeof(T), m_dwParam);
	if (nIndex < m_nCount)
		MoveItem(&m_pArray[nIndex + 1], &m_pArray[nIndex], m_dwParam, m_nCount - nIndex);
	NewItem(&m_pArray[nIndex], m_dwParam, 1);
	MoveItem(&m_pArray[nIndex], (T*) &item, m_dwParam, 1, true);
	m_nCount++;
	return TRUE;
}

template <class T, class ARG_T> BOOL CMySimpleArrayT<T, ARG_T>::InsertItem(int nIndex)
{
	if (nIndex < 0 || nIndex > m_nCount)
	{
		ArrayDebugBreak();
		return FALSE;
	}

	if (m_pArray == NULL)
		m_pArray = AllocMemory(sizeof(T) * (m_nMemCount = (m_nCount + 1)), m_dwParam);
	else if (m_nCount >= m_nMemCount)
		m_pArray = ReAllocMemory(m_pArray, (m_nMemCount = (m_nCount + 1)) * sizeof(T), m_dwParam);
	if (nIndex < m_nCount)
		MoveItem(&m_pArray[nIndex + 1], &m_pArray[nIndex], m_dwParam, m_nCount - nIndex);
	NewItem(&m_pArray[nIndex], m_dwParam, 1);
	m_nCount++;
	return TRUE;
}

template <class T, class ARG_T> inline T CMySimpleArrayT<T, ARG_T>::GetItem(int nIndex) const
{
	if (nIndex < 0 || nIndex >= m_nCount)
		ArrayDebugBreak();
	return m_pArray[nIndex];
}

template <class T, class ARG_T> inline T& CMySimpleArrayT<T, ARG_T>::GetItemRef(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_nCount)
		ArrayDebugBreak();
	return m_pArray[nIndex];
}

template <class T, class ARG_T> inline T* CMySimpleArrayT<T, ARG_T>::GetItemPtr(int nIndex) const
{
	if (nIndex < 0 || nIndex >= m_nCount)
		ArrayDebugBreak();
	return &m_pArray[nIndex];
}

template <class T, class ARG_T> inline int CMySimpleArrayT<T, ARG_T>::GetCount() const
{
	return m_nCount;
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::SetCount(int nCount)
{
	if (nCount == m_nCount)
		return;
	else if (nCount <= m_nCount)
	{
		if (nCount == 0)
			RemoveAll();
		else
		{
			// ó]åvÇ»çÄñ⁄ÇÕÉfÅ[É^ÇÃÇ›çÌèú
			DeleteItem(&m_pArray[nCount], m_dwParam, m_nCount - nCount);
		}
	}
	else
	{
		if (nCount > m_nMemCount)
		{
			m_pArray = ReAllocMemory(m_pArray, nCount * sizeof(T), m_dwParam);
			m_nMemCount = nCount;
		}
		// Ç¢Ç¬Ç≈Ç‡égÇ¶ÇÈÇÊÇ§Ç…èâä˙âªÇµÇƒÇ®Ç≠
		NewItem(&m_pArray[m_nCount], m_dwParam, nCount - m_nCount);
	}
	m_nCount = nCount;
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::GrowArray(int nCount)
{
	if (nCount <= m_nMemCount)
		return;
	m_pArray = ReAllocMemory(m_pArray, nCount * sizeof(T), m_dwParam);
	m_nMemCount = nCount;
}

// use quick-sort
template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::Sort()
{
	if (m_nCount < 2)
		return;

	QuickSort(m_pArray, m_nCount, m_pTemp, m_dwParam);
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::RemoveItem(int nIndex, bool bNoDel)
{
	if (m_nCount == 0)
		return;
	if (nIndex < 0 || nIndex >= m_nCount)
	{
		ArrayDebugBreak();
		return;
	}
	if (m_nCount == 1)
	{
		if (!bNoDel)
			DeleteItem(m_pArray, m_dwParam, m_nCount);
		//FreeMemory(m_pArray);
		//m_pArray = NULL;
	}
	else
	{
		if (!bNoDel)
			DeleteItem(&m_pArray[nIndex], m_dwParam);
		MoveItem(&m_pArray[nIndex], &m_pArray[nIndex + 1], m_dwParam, m_nCount - (nIndex + 1));
		//m_pArray = ReAllocMemory(m_pArray, sizeof(T) * (m_nCount - 1), m_dwParam);
	}
	m_nCount--;
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::RemoveAll()
{
	if (m_nCount == 0)
		return;
	DeleteItem(m_pArray, m_dwParam, m_nCount);
	//FreeMemory(m_pArray, m_dwParam);
	//m_pArray = NULL;
	m_nCount = 0;
	//m_nMemCount = 0;
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::RemoveAllCompletely()
{
	if (m_nMemCount == 0)
		return;
	DeleteItem(m_pArray, m_dwParam, m_nCount);
	FreeMemory(m_pArray, m_dwParam);
	m_pArray = NULL;
	m_nCount = 0;
	m_nMemCount = 0;
}

template <class T, class ARG_T> BOOL CMySimpleArrayT<T, ARG_T>::SetItem(int nIndex, ARG_T item)
{
	if (nIndex == m_nCount)
		return Add(item) != -1;
	else if (nIndex < 0 || nIndex > m_nCount)
	{
		ArrayDebugBreak();
		return FALSE;
	}
	DeleteItem(&m_pArray[nIndex], m_dwParam);
	MoveItem(&m_pArray[nIndex], (T*) &item, m_dwParam, 1, true);
	return TRUE;
}

template <class T, class ARG_T> int CMySimpleArrayT<T, ARG_T>::FindItem(ARG_T item) const
{
	for (int n = 0; n < m_nCount; n++)
	{
		if (CompareItem(&m_pArray[n], (T*) &item, m_dwParam) == 0)
			return n;
	}
	return -1;
}

template <class T, class ARG_T> int CMySimpleArrayT<T, ARG_T>::GetIndexFromPtr(T* ptr) const
{
	for (int n = 0; n < m_nCount; n++)
	{
		if (&m_pArray[n] == ptr)
			return n;
	}
	return -1;
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::ChangePlace(int nIndex1, int nIndex2)
{
	if (nIndex1 < 0 || nIndex1 >= m_nCount || nIndex2 < 0 || nIndex2 >= m_nCount)
		ArrayDebugBreak();
	memcpy(m_pTemp, &m_pArray[nIndex1], sizeof(T));
	memcpy(&m_pArray[nIndex1], &m_pArray[nIndex2], sizeof(T));
	memcpy(&m_pArray[nIndex2], m_pTemp, sizeof(T));
#ifdef _DEBUG
	memset(m_pTemp, 0xFF, sizeof(T));
#endif
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::SlideItem(int nIndex, int nNewPlace)
{
	if (nIndex < 0 || nIndex >= m_nCount || nNewPlace < 0 || nNewPlace >= m_nCount)
		ArrayDebugBreak();
	memcpy(m_pTemp, &m_pArray[nIndex], sizeof(T));
	if (nIndex < nNewPlace)
		memmove(&m_pArray[nIndex], &m_pArray[nIndex + 1], sizeof(T) * (nNewPlace - nIndex));
	else
		memmove(&m_pArray[nNewPlace + 1], &m_pArray[nNewPlace], sizeof(T) * (nIndex - nNewPlace));
	memcpy(&m_pArray[nNewPlace], m_pTemp, sizeof(T));
#ifdef _DEBUG
	memset(m_pTemp, 0xFF, sizeof(T));
#endif
}

template <class T, class ARG_T> bool CMySimpleArrayT<T, ARG_T>::SortItems(const int* pNewIndexArray)
{
	T* pTempArray;
	int n;
	if (m_nCount <= 1)
		return m_nCount == 1;
	if (!pNewIndexArray)
		return false;
	pTempArray = AllocMemory(sizeof(T) * m_nCount, m_dwParam);
	if (!pTempArray)
		return false;
	for (n = 0; n < m_nCount; n++)
	{
		if (pNewIndexArray[n] < 0 || pNewIndexArray[n] >= m_nCount)
			ArrayDebugBreak();
		memcpy(&pTempArray[n], &m_pArray[pNewIndexArray[n]], sizeof(T));
	}
	memcpy(m_pArray, pTempArray, sizeof(T) * m_nCount);
	FreeMemory(pTempArray, m_dwParam);
	return true;
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::MoveArray(CMySimpleArrayT<T, ARG_T>& rArray)
{
	RemoveAll();
	if (rArray.m_nCount == 0)
		return;
	if (m_nMemCount)
		FreeMemory(m_pArray, m_dwParam);
	m_nCount = rArray.m_nCount;
	m_nMemCount = rArray.m_nMemCount;
	m_pArray = rArray.m_pArray;
	m_dwParam = rArray.m_dwParam;
	rArray.m_nCount = 0;
	rArray.m_nMemCount = 0;
	rArray.m_pArray = NULL;
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::CopyArray(const CMySimpleArrayT<T, ARG_T>& rArray)
{
	RemoveAll();
	if (rArray.m_nCount == 0)
		return;
	m_nCount = rArray.m_nCount;
	m_dwParam = rArray.m_dwParam;
	if (m_pArray == NULL)
		m_pArray = AllocMemory(sizeof(T) * (m_nMemCount = m_nCount), m_dwParam);
	else if (m_nCount > m_nMemCount)
		m_pArray = ReAllocMemory(m_pArray, (m_nMemCount = m_nCount) * sizeof(T), m_dwParam);
	NewItem(m_pArray, m_dwParam, m_nCount);
	MoveItem(m_pArray, rArray.m_pArray, m_dwParam, m_nCount, true);
}

template <class T, class ARG_T> int CMySimpleArrayT<T, ARG_T>::AddArray(const CMySimpleArrayT<T, ARG_T>& rArray)
{
	if (rArray.m_nCount == 0)
		return m_nCount;
	if (m_pArray == NULL)
	{
		m_pArray = AllocMemory(sizeof(T) * (rArray.m_nCount), m_dwParam);
		m_nMemCount = rArray.m_nCount;
	}
	else if (m_nCount + rArray.m_nCount > m_nMemCount)
		m_pArray = ReAllocMemory(m_pArray, (m_nMemCount = (m_nCount + rArray.m_nCount)) * sizeof(T), m_dwParam);
	NewItem(&m_pArray[m_nCount], m_dwParam, rArray.m_nCount);
	MoveItem(&m_pArray[m_nCount], rArray.m_pArray, m_dwParam, rArray.m_nCount, true);
	m_nCount += rArray.m_nCount;
	return m_nCount;
}

template <class T, class ARG_T> int CMySimpleArrayT<T, ARG_T>::InsertArray(int nIndex, const CMySimpleArrayT<T, ARG_T>& rArray)
{
	if (nIndex < 0 || nIndex > m_nCount)
	{
		ArrayDebugBreak();
		return -1;
	}
	if (rArray.m_nCount == 0)
		return m_nCount;
	if (m_pArray == NULL)
		m_pArray = AllocMemory(sizeof(T) * (rArray.m_nCount), m_dwParam);
	else if (m_nCount + rArray.m_nCount > m_nMemCount)
		m_pArray = ReAllocMemory(m_pArray, (m_nMemCount = (m_nCount + rArray.m_nCount)) * sizeof(T), m_dwParam);
	if (nIndex < m_nCount)
		MoveItem(&m_pArray[nIndex + rArray.m_nCount], &m_pArray[nIndex], m_dwParam, m_nCount - nIndex);
	NewItem(&m_pArray[nIndex], m_dwParam, rArray.m_nCount);
	MoveItem(&m_pArray[nIndex], rArray.m_pArray, m_dwParam, rArray.m_nCount, true);
	m_nCount += rArray.m_nCount;
	return m_nCount;
}

template <class T, class ARG_T> void CMySimpleArrayT<T, ARG_T>::SetArray(T* pArray, int nCount)
{
	RemoveAll();
	if (nCount == 0)
		return;
	if (m_pArray)
		FreeMemory(m_pArray, m_dwParam);
	m_nCount = nCount;
	m_nMemCount = nCount;
	m_pArray = pArray;
}

template <class T, class ARG_T> inline int CMySimpleArrayT<T, ARG_T>::Push(ARG_T item)
{
	return Add(item);
}

template <class T, class ARG_T> inline bool CMySimpleArrayT<T, ARG_T>::Pop(T* pRet)
{
	if (m_nCount == 0)
		return false;
	MoveItem(pRet, &m_pArray[--m_nCount], m_dwParam, 1, true);
	DeleteItem(&m_pArray[m_nCount], m_dwParam, 1);
	return true;
}

template <class T, class ARG_T> inline void* CMySimpleArrayT<T, ARG_T>::GetFirstStackPosition() const
{
	if (m_nCount == 0)
		return NULL;
	return (void*) IntToPtr(m_nCount);
}

template <class T, class ARG_T> inline bool CMySimpleArrayT<T, ARG_T>::GetNextStackPosition(void*& pos, T* pRet, int* pnIndex) const
{
	if (!pos)
		return false;
	int i = PtrToInt(pos) - 1;
	if (pnIndex)
		*pnIndex = i;
	MoveItem(pRet, &m_pArray[i--], m_dwParam, 1, true);
	pos = (void*) IntToPtr(i + 1);
	return true;
}

template <class T, class ARG_T> inline int CMySimpleArrayT<T, ARG_T>::Enqueue(ARG_T item)
{
	return Add(item);
}

template <class T, class ARG_T> inline bool CMySimpleArrayT<T, ARG_T>::Dequeue(T* pRet)
{
	if (m_nCount == 0)
		return false;
	MoveItem(pRet, &m_pArray[0], m_dwParam, 1, true);
	DeleteItem(&m_pArray[0], m_dwParam, 1);
	MoveItem(&m_pArray[0], &m_pArray[1], m_dwParam, m_nCount - 1);
	m_nCount--;
	return true;
}

template <class T, class ARG_T> inline void* CMySimpleArrayT<T, ARG_T>::GetFirstQueuePosition() const
{
	if (m_nCount == 0)
		return NULL;
	return (void*) IntToPtr(1);
}

template <class T, class ARG_T> inline bool CMySimpleArrayT<T, ARG_T>::GetNextQueuePosition(void*& pos, T* pRet, int* pnIndex) const
{
	if (!pos)
		return false;
	int i = PtrToInt(pos) - 1;
	if (pnIndex)
		*pnIndex = i;
	MoveItem(pRet, &m_pArray[i++], m_dwParam, 1, true);
	if (i >= m_nCount)
		pos = NULL;
	else
		pos = (void*) IntToPtr(i + 1);
	return true;
}

template <class T, class ARG_T> inline T CMySimpleArrayT<T, ARG_T>::operator [](int nIndex) const
{
	if (nIndex < 0 || nIndex >= m_nCount)
		ArrayDebugBreak();
	return m_pArray[nIndex];
}

template <class T, class ARG_T> inline T& CMySimpleArrayT<T, ARG_T>::operator [](int nIndex)
{
	if (nIndex < 0 || nIndex >= m_nCount)
		ArrayDebugBreak();
	return m_pArray[nIndex];
}

template <class T, class ARG_T> void __stdcall CMySimpleArrayT<T, ARG_T>::_QuickSortInner(T* pArray, int left, int right, T* pTemp, DWORD dwParam,
	int (__stdcall* pfnCompareItem)(const T*, const T*, DWORD))
{
	if (right - left < 1)
		return;
	int l, r, c, i, j;
	c = (left + right + 1) / 2;
	T* p = &pArray[c];
	l = left;
	r = right;
	while (true)
	{
		for (i = l; i <= right; i++)
		{
			if (pfnCompareItem(&pArray[i], p, dwParam) >= 0)
				break;
		}
		for (j = r; j >= left; j--)
		{
			if (pfnCompareItem(p, &pArray[j], dwParam) >= 0)
				break;
		}
		if (i < j)
		{
			memcpy(pTemp, &pArray[i], sizeof(T));
			memcpy(&pArray[i], &pArray[j], sizeof(T));
			memcpy(&pArray[j], pTemp, sizeof(T));
			if (i == c)
			{
				p = &pArray[j];
				c = j;
			}
			else if (j == c)
			{
				p = &pArray[i];
				c = i;
			}
			l = i + 1;
			r = j - 1;
			continue;
		}
//#ifdef _DEBUG
//		_DumpArray(&pArray[left], i - left, dwParam);
//		_DumpArray(&pArray[i], right - i + 1, dwParam);
//#endif
		if (left < i)
			_QuickSortInner(pArray, left, i - 1, pTemp, dwParam, pfnCompareItem);
		if (right > i)
			_QuickSortInner(pArray, i, right, pTemp, dwParam, pfnCompareItem);
		break;
	}
}

template <class T, class ARG_T> void __stdcall CMySimpleArrayT<T, ARG_T>::QuickSort(T* pArray, int nCount, T* pTemp, DWORD dwParam)
{
	_QuickSortInner(pArray, 0, nCount - 1, pTemp, dwParam, CompareItem);
}

template <class T, class ARG_T> void __stdcall DoQuickSortArray(CMySimpleArrayT<T, ARG_T>& arrData)
{
	CMySimpleArrayT<T, ARG_T>::_QuickSortInner(arrData.m_pArray, 0, arrData.m_nCount - 1, arrData.m_pTemp, arrData.m_dwParam,
		CMySimpleArrayT<T, ARG_T>::CompareItem);
}

template <class T, class ARG_T> void __stdcall DoQuickSortArrayEx(CMySimpleArrayT<T, ARG_T>& arrData,
	int (__stdcall* pfnCompareItem)(const T*, const T*, DWORD))
{
	CMySimpleArrayT<T, ARG_T>::_QuickSortInner(arrData.m_pArray, 0, arrData.m_nCount - 1, arrData.m_pTemp, arrData.m_dwParam,
		pfnCompareItem);
}

#ifdef _DEBUG
template <class T, class ARG_T> void __stdcall CMySimpleArrayT<T, ARG_T>::_DumpArray(T* pArray, int nCount, DWORD dwParam)
{
	TCHAR szBuffer[30];
	_stprintf_s(szBuffer, _T("Count: %d\n"), nCount);
	OutputDebugString(szBuffer);
}
#endif



template <class T>
class CMySimpleArray : public CMySimpleArrayT<T, T>
{
public:
	CMySimpleArray() { }
	CMySimpleArray(const CMySimpleArray<T>& arr) : CMySimpleArrayT<T, T>::CMySimpleArrayT(arr) { }
	CMySimpleArray(T* arr, int nCount) : CMySimpleArrayT<T, T>::CMySimpleArrayT(arr, nCount) { }
};

typedef CMySimpleArrayT<void*, void*> CMyPtrArray;

template <class T>
class CMyPtrArrayT : public CMyPtrArray
{
public:
	inline CMyPtrArrayT() { }
	inline CMyPtrArrayT(const CMyPtrArrayT<T>& arr) : CMyPtrArray(arr) { }
	inline CMyPtrArrayT(const T** arr, int count) : CMyPtrArray((const void**) arr, count) { }

public:
	inline int Add(T* item) { return CMyPtrArray::Add(item); }
	inline int Add() { return CMyPtrArray::Add(); }
	inline BOOL InsertItem(int nIndex, T* item) { return CMyPtrArray::InsertItem(nIndex, item); }
	inline BOOL InsertItem(int nIndex) { return CMyPtrArray::InsertItem(nIndex); }
	inline T* GetItem(int nIndex) const { return (T*) CMyPtrArray::GetItem(nIndex); }
	inline T*& GetItemRef(int nIndex) { return (T*&) CMyPtrArray::GetItem(nIndex); }
	inline T** GetItemPtr(int nIndex) const { return (T**) CMyPtrArray::GetItemPtr(nIndex); }
	inline BOOL SetItem(int nIndex, T* item) { return CMyPtrArray::SetItem(nIndex, item); }
	inline int FindItem(T* item) const { return CMyPtrArray::FindItem(item); }
	inline int GetIndexFromPtr(T** ptr) const { return CMyPtrArray::GetIndexFromPtr((void**) ptr); }
	inline void MoveArray(CMyPtrArrayT<T>& rArray) { CMyPtrArray::MoveArray(rArray); }
	inline void CopyArray(const CMyPtrArrayT<T>& rArray) { CMyPtrArray::CopyArray(rArray); }
	inline int AddArray(const CMyPtrArrayT<T>& rArray) { return CMyPtrArray::AddArray(rArray); }
	inline int InsertArray(int nIndex, const CMyPtrArrayT<T>& rArray) { return CMyPtrArray::InsertArray(nIndex, rArray); }
	inline void SetArray(T** pArray, int nCount) { CMyPtrArray::SetArray((void**) pArray, nCount); }
	inline T* operator [] (int nIndex) const { return (T*) CMyPtrArray::operator [](nIndex); }
	inline T*& operator [] (int nIndex) { return (T*&) CMyPtrArray::operator [](nIndex); }

	inline int Push(T* item) { return CMyPtrArray::Push(item); }
	inline bool Pop(T** pRet) { return CMyPtrArray::Pop((void**) pRet); }
	inline bool GetNextStackPosition(void*& pos, T** pRet, int* pnIndex = NULL) const
		{ return CMyPtrArray::GetNextStackPosition(pos, (void**) pRet, pnIndex); }
	inline int Enqueue(T* item) { return CMyPtrArray::Enqueue(item); }
	inline bool Dequeue(T** pRet) { return CMyPtrArray::Dequeue((void**) pRet); }
	inline bool GetNextQueuePosition(void*& pos, T** pRet, int* pnIndex = NULL) const
		{ return CMyPtrArray::GetNextQueuePosition(pos, (void**) pRet, pnIndex); }
};

template <class PTR_T>
class CMyPtrArrayPtrT : public CMyPtrArray
{
public:
	inline CMyPtrArrayPtrT() { }
	inline CMyPtrArrayPtrT(const CMyPtrArrayPtrT<PTR_T>& arr) : CMyPtrArray(arr) { }
	inline CMyPtrArrayPtrT(const PTR_T* arr, int count) : CMyPtrArray((const void**) arr, count) { }

public:
	inline int Add(PTR_T item) { return CMyPtrArray::Add(item); }
	inline int Add() { return CMyPtrArray::Add(); }
	inline BOOL InsertItem(int nIndex, PTR_T item) { return CMyPtrArray::InsertItem(nIndex, item); }
	inline BOOL InsertItem(int nIndex) { return CMyPtrArray::InsertItem(nIndex); }
	inline PTR_T GetItem(int nIndex) const { return (PTR_T) CMyPtrArray::GetItem(nIndex); }
	inline PTR_T& GetItemRef(int nIndex) { return (PTR_T&) CMyPtrArray::GetItem(nIndex); }
	inline PTR_T* GetItemPtr(int nIndex) const { return (PTR_T*) CMyPtrArray::GetItemPtr(nIndex); }
	inline BOOL SetItem(int nIndex, PTR_T item) { return CMyPtrArray::SetItem(nIndex, item); }
	inline int FindItem(PTR_T item) const { return CMyPtrArray::FindItem(item); }
	inline int GetIndexFromPtr(PTR_T* ptr) const { return CMyPtrArray::GetIndexFromPtr((void**) ptr); }
	inline void MoveArray(CMyPtrArrayPtrT<PTR_T>& rArray) { CMyPtrArray::MoveArray(rArray); }
	inline void CopyArray(const CMyPtrArrayPtrT<PTR_T>& rArray) { CMyPtrArray::CopyArray(rArray); }
	inline int AddArray(const CMyPtrArrayPtrT<PTR_T>& rArray) { return CMyPtrArray::AddArray(rArray); }
	inline int InsertArray(int nIndex, const CMyPtrArrayPtrT<PTR_T>& rArray) { return CMyPtrArray::InsertArray(nIndex, rArray); }
	inline void SetArray(PTR_T* pArray, int nCount) { CMyPtrArray::SetArray((void**) pArray, nCount); }
	inline PTR_T operator [] (int nIndex) const { return (PTR_T) CMyPtrArray::operator [](nIndex); }
	inline PTR_T& operator [] (int nIndex) { return (PTR_T&) CMyPtrArray::operator [](nIndex); }

	inline int Push(PTR_T item) { return CMyPtrArray::Push(item); }
	inline bool Pop(PTR_T* pRet) { return CMyPtrArray::Pop((void**) pRet); }
	inline bool GetNextStackPosition(void*& pos, PTR_T* pRet, int* pnIndex = NULL) const
		{ return CMyPtrArray::GetNextStackPosition(pos, (void**) pRet, pnIndex); }
	inline int Enqueue(PTR_T item) { return CMyPtrArray::Enqueue(item); }
	inline bool Dequeue(PTR_T* pRet) { return CMyPtrArray::Dequeue((void**) pRet); }
	inline bool GetNextQueuePosition(void*& pos, PTR_T* pRet, int* pnIndex = NULL) const
		{ return CMyPtrArray::GetNextQueuePosition(pos, (void**) pRet, pnIndex); }
};

class CMyStringArrayA : public CMySimpleArrayT<LPSTR, LPCSTR>
{
public:
	CMyStringArrayA();
	CMyStringArrayA(const CMyStringArrayA& arr) : CMySimpleArrayT(arr) { }
	CMyStringArrayA(const LPCSTR* arr, int nCount) : CMySimpleArrayT((const LPSTR*) arr, nCount) { }
	//int Add2(LPCSTR lpsz)
	//	{ return Add((LPSTR) lpsz); }
	void SetCaseSensitive(bool bCaseSensitive);
};

class CMyStringArrayW : public CMySimpleArrayT<LPWSTR, LPCWSTR>
{
public:
	CMyStringArrayW();
	CMyStringArrayW(const CMyStringArrayW& arr) : CMySimpleArrayT(arr) { }
	CMyStringArrayW(const LPCWSTR* arr, int nCount) : CMySimpleArrayT((const LPWSTR*) arr, nCount) { }
	//int Add2(LPCWSTR lpsz)
	//	{ return Add((LPWSTR) lpsz); }
	void SetCaseSensitive(bool bCaseSensitive);
};

#ifdef _UNICODE
typedef CMyStringArrayW CMyStringArray;
#else
typedef CMyStringArrayA CMyStringArray;
#endif

inline CMyStringArrayA::CMyStringArrayA()
{
	*m_pTemp = NULL;
	m_dwParam = 1;
}

inline void CMyStringArrayA::SetCaseSensitive(bool bCaseSensitive)
{
	m_dwParam = bCaseSensitive ? 1 : 0;
}

template <>
inline void __stdcall CMySimpleArrayT<LPSTR, LPCSTR>::NewItem(LPSTR* p, DWORD dwParam, int nCount)
{
	while (nCount--)
		*p++ = NULL;
}

template <>
inline void __stdcall CMySimpleArrayT<LPSTR, LPCSTR>::CloneToItem(LPSTR* pDest, const LPSTR* pSrc, DWORD dwParam)
{
	if (*pSrc)
		*pDest = _strdup(*pSrc);
	else
		*pDest = NULL;
}

template <>
inline int __stdcall CMySimpleArrayT<LPSTR, LPCSTR>::CompareItem(const LPSTR* p1, const LPSTR* p2, DWORD dwParam)
{
	if (*p1 && !*p2)
		return 1;
	else if (!*p1 && *p2)
		return -1;
	else if (!*p1 && !*p2)
		return 0;
	return (dwParam ? strcmp(*p1, *p2) : _stricmp(*p1, *p2));
}

template <>
inline void __stdcall CMySimpleArrayT<LPSTR, LPCSTR>::DeleteItem(LPSTR* p, DWORD dwParam, int nCount)
{
	while (nCount--)
	{
		if (*p)
			free(*p);
		p++;
	}
}

inline CMyStringArrayW::CMyStringArrayW()
{
	*m_pTemp = NULL;
	m_dwParam = 1;
}

inline void CMyStringArrayW::SetCaseSensitive(bool bCaseSensitive)
{
	m_dwParam = bCaseSensitive ? 1 : 0;
}

template <>
inline void __stdcall CMySimpleArrayT<LPWSTR, LPCWSTR>::NewItem(LPWSTR* p, DWORD dwParam, int nCount)
{
	while (nCount--)
		*p++ = NULL;
}

template <>
inline void __stdcall CMySimpleArrayT<LPWSTR, LPCWSTR>::CloneToItem(LPWSTR* pDest, const LPWSTR* pSrc, DWORD dwParam)
{
	if (*pSrc)
		*pDest = _wcsdup(*pSrc);
	else
		*pDest = NULL;
}

template <>
inline int __stdcall CMySimpleArrayT<LPWSTR, LPCWSTR>::CompareItem(const LPWSTR* p1, const LPWSTR* p2, DWORD dwParam)
{
	if (*p1 && !*p2)
		return 1;
	else if (!*p1 && *p2)
		return -1;
	else if (!*p1 && !*p2)
		return 0;
	return (dwParam ? wcscmp(*p1, *p2) : _wcsicmp(*p1, *p2));
}

template <>
inline void __stdcall CMySimpleArrayT<LPWSTR, LPCWSTR>::DeleteItem(LPWSTR* p, DWORD dwParam, int nCount)
{
	while (nCount--)
	{
		if (*p)
			free(*p);
		p++;
	}
}

#ifdef NEW_MACRO_PUSHED_WITH_MSC
#undef NEW_MACRO_PUSHED_WITH_MSC
#pragma pop_macro("new")
#endif

#endif // __ARRAY_H__
