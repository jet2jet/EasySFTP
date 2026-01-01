/*
 Copyright (C) 2010 jet (ジェット)

 KeyList.h - declarations of CMyKeyList and CMyStringKeyList[A/W]
 */

#ifndef __KEYLIST_H__
#define __KEYLIST_H__

#pragma once

#define IsAddress(var, size) (HIWORD(var) != 0)

typedef void* _INDEX;

template<class TYPE, class KEY, KEY DefKey = NULL>
class CMyKeyList
{
protected:
	struct CMyKeyListItem
	{
		_INDEX unused;
		CMyKeyListItem* pNext;
		KEY key;
		TYPE value;
	};

	void Construct()
	{
		m_nCount = 0;
		m_pFirst = NULL;
		memset(&m_empty, 0, sizeof(m_empty));
		m_empty.pNext = NULL;
	}

public:
	CMyKeyList()
		{ Construct(); }

	int GetCount() const
		{ return m_nCount; }

	BOOL IsEmpty() const
		{ return m_nCount == 0; }

	void Remove(int nIndex)
	{
		CMyKeyListItem* pItem = m_pFirst;
		CMyKeyListItem* pPrev = NULL;
		for (int n = 0; n < m_nCount && pItem != NULL; n++)
		{
			if (n == nIndex)
				break;
			pPrev = pItem;
			pItem = pItem->pNext;
		}
		if (pItem == NULL)
			return;
		if (pPrev != NULL)
			pPrev->pNext = pItem->pNext;
		if (m_pFirst == pItem)
			m_pFirst = pItem->pNext;
		m_nCount--;
		DeleteItem(pItem);
	}

	void Remove(KEY key)
	{
		int n = IndexFromKey(key);
		if (n == -1)
			return;
		Remove((_INDEX) IntToPtr(n));
	}

	void Remove(_INDEX aIndex)
	{
		CMyKeyListItem* pItem = NULL;
		CMyKeyListItem* pPrev = NULL;
		if (IsAddress(aIndex, sizeof(_INDEX)))
		{
			pItem = m_pFirst;
			for (int n = 0; n < m_nCount; n++)
			{
				if (pItem == (CMyKeyListItem*) aIndex)
					break;
				pPrev = pItem;
				pItem = pItem->pNext;
			}
			pItem = (CMyKeyListItem*) aIndex;
		}
		else
		{
			int nIndex = (int) PtrToInt(aIndex);
			pItem = m_pFirst;
			for (int n = 0; n < m_nCount && pItem != NULL; n++)
			{
				if (n == nIndex)
					break;
				pPrev = pItem;
				pItem = pItem->pNext;
			}
		}
		if (pItem == NULL)
			return;
		if (pPrev != NULL)
			pPrev->pNext = pItem->pNext;
		if (m_pFirst == pItem)
			m_pFirst = pItem->pNext;
		m_nCount--;
		DeleteItem(pItem);
	}

	void RemoveAll()
	{
		CMyKeyListItem* pItem = m_pFirst;
		CMyKeyListItem* pDel;
		while (pItem != NULL)
		{
			pDel = pItem;
			pItem = pItem->pNext;
			DeleteItem(pDel);
		}
		m_pFirst = NULL;
		m_nCount = 0;
	}

	_INDEX Add(TYPE value, KEY key = DefKey)
	{
		if (m_nCount == 65535)
			return NULL;
		CMyKeyListItem* pItem = CreateItem(value, key);
		if (m_pFirst == NULL)
			m_pFirst = pItem;
		else
		{
			CMyKeyListItem* pLast = GetLastItem();
			pLast->pNext = pItem;
		}
		m_nCount++;
		return (_INDEX) pItem;
	}

	TYPE GetItem(KEY key) const
	{
		int n = IndexFromKey(key);
		if (n == -1)
			return m_empty.value;
		return GetItem((_INDEX) IntToPtr(n));
	}

	TYPE GetItem(int nIndex) const
	{
		CMyKeyListItem* pItem = m_pFirst;
		for (int n = 0; n < m_nCount && pItem != NULL; n++)
		{
			if (n == nIndex)
				return pItem->value;
			pItem = pItem->pNext;
		}

		return m_empty.value;
	}

	TYPE GetItem(_INDEX ind) const
	{
		if (IsAddress(ind, sizeof(_INDEX)))
		{
			CMyKeyListItem* pItem = (CMyKeyListItem*) ind;
			return pItem->value;
		}
		else
		{
			int nIndex = (int) PtrToInt(ind);
			CMyKeyListItem* pItem = m_pFirst;
			for (int n = 0; n < m_nCount && pItem != NULL; n++)
			{
				if (n == nIndex)
					return pItem->value;
				pItem = pItem->pNext;
			}

			return m_empty.value;
		}
	}

	TYPE& GetItemVar(KEY key)
	{
		int n = IndexFromKey(key);
		if (n == -1)
			return m_empty.value;
		return GetItemVar((_INDEX) n);
	}

	TYPE& GetItemVar(int nIndex)
	{
		CMyKeyListItem* pItem = m_pFirst;
		for (int n = 0; n < m_nCount && pItem != NULL; n++)
		{
			if (n == nIndex)
				return pItem->value;
			pItem = pItem->pNext;
		}

		return m_empty.value;
	}

	TYPE& GetItemVar(_INDEX ind)
	{
		if (IsAddress(ind, sizeof(_INDEX)))
		{
			CMyKeyListItem* pItem = (CMyKeyListItem*) ind;
			return pItem->value;
		}
		else
		{
			int nIndex = (int) ind;
			CMyKeyListItem* pItem = m_pFirst;
			for (int n = 0; n < m_nCount && pItem != NULL; n++)
			{
				if (n == nIndex)
					return pItem->value;
				pItem = pItem->pNext;
			}

			return m_empty.value;
		}
	}

	KEY IndexToKey(int nIndex) const
	{
		CMyKeyListItem* pItem = GetItemData(nIndex);
		if (pItem == NULL)
			return DefKey;
		return pItem->key;
	}

	KEY IndexToKey(_INDEX aIndex) const
	{
		if (IsAddress(aIndex, sizeof(_INDEX)))
			return ((CMyKeyListItem*) aIndex)->key;
		return IndexToKey((int) PtrToInt(aIndex));
	}

	int IndexFromKey(KEY key) const
	{
		CMyKeyListItem* pItem = m_pFirst;
		int n = 0;
		while (pItem != NULL)
		{
			if (CompareKey(key, pItem->key))
				return n;
			n++;
			pItem = pItem->pNext;
		}
		return -1;
	}

	BOOL IsItemKey(KEY key) const
	{
		CMyKeyListItem* pItem = m_pFirst;
		while (pItem != NULL)
		{
			if (CompareKey(key, pItem->key))
				return TRUE;
			pItem = pItem->pNext;
		}
		return FALSE;
	}

	_INDEX InsertItem(int nIndex, TYPE value, KEY key = DefKey)
	{
		if (nIndex > m_nCount || m_nCount == 65535)
			return NULL;
		CMyKeyListItem* pItem = CreateItem(value, key);
		CMyKeyListItem* pBefore = NULL;
		if (nIndex == m_nCount)
		{
			if (m_nCount == 0)
				m_pFirst = pItem;
			else
			{
				pBefore = GetLastItem();
				pBefore->pNext = pItem;
			}
		}
		else
		{
			pBefore = GetItemData(nIndex);
			if (pBefore != NULL)
			{
				CMyKeyListItem* pPrev = GetPrevItem(pBefore);
				if (pPrev != NULL)
					pPrev->pNext = pItem;
				pItem->pNext = pBefore;
			}
			else
				return NULL;
		}
		m_nCount++;
		return (_INDEX) pItem;
	}

	BOOL SetItem(int nIndex, TYPE value, KEY NewKey = DefKey)
	{
		CMyKeyListItem* pItem = GetItemData(nIndex);
		if (pItem == NULL)
			return FALSE;
		pItem->value = value;
		if (NewKey != DefKey)
		{
			DestructKey(&pItem->key);
			pItem->key = CreateCopyKey(NewKey);
		}
		return TRUE;
	}

	BOOL SetItem(KEY key, TYPE value, KEY NewKey = DefKey)
	{
		int n = IndexFromKey(key);
		if (n == -1)
			return FALSE;
		return SetItem((_INDEX) IntToPtr(n), value, NewKey);
	}

	BOOL SetItem(_INDEX ind, TYPE value, KEY NewKey = DefKey)
	{
		if (IsAddress(ind, sizeof(_INDEX)))
		{
			CMyKeyListItem* pItem = (CMyKeyListItem*) ind;
			pItem->value = value;
			if (NewKey != DefKey)
			{
				DestructKey(&pItem->key);
				pItem->key = CreateCopyKey(NewKey);
			}
			return TRUE;
		}
		else
		{
			int nIndex = (int) ind;
			CMyKeyListItem* pItem = GetItemData(nIndex);
			if (pItem == NULL)
				return FALSE;
			pItem->value = value;
			if (NewKey != DefKey)
			{
				DestructKey(&pItem->key);
				pItem->key = CreateCopyKey(NewKey);
			}
			return TRUE;
		}
	}

	BOOL SetItemEx(KEY key, TYPE value)
	{
		if (IsItemKey(key))
			return SetItem(key, value);
		else
			return Add(value, key) != NULL;
	}

	_INDEX Find(TYPE value, _INDEX start = NULL)
	{
		CMyKeyListItem* pItem;
		if (start == NULL)
			pItem = m_pFirst;
		else if (IsAddress(start, sizeof(_INDEX)))
			pItem = (CMyKeyListItem*) start;
		else
			pItem = GetItemData((int) start);
		while (pItem != NULL)
		{
			if (pItem->value == value)
				return (_INDEX) pItem;
		}
		return NULL;
	}

	_INDEX PositionFromKey(KEY key)
	{
		CMyKeyListItem* pItem = m_pFirst;
		while (pItem != NULL)
		{
			if (CompareKey(pItem->key, key))
				return (_INDEX) pItem;
			pItem = pItem->pNext;
		}
		return NULL;
	}

	_INDEX GetFirstItemPosition() const
	{
		return (_INDEX) m_pFirst;
	}

	int GetNextItem(_INDEX& rIndex, TYPE& rValue, KEY& rKey) const
	{
		CMyKeyListItem* pItem = (CMyKeyListItem*) rIndex;
		if (pItem == NULL)
			return -1;
		rIndex = (_INDEX) pItem->pNext;
		rValue = pItem->value;
		rKey = CreateCopyKey(pItem->key);
		if (rIndex == NULL)
			return 0;
		return 1;
	}

	TYPE GetNextItem(_INDEX& rIndex, KEY* pKey = NULL) const
	{
		CMyKeyListItem* pItem = (CMyKeyListItem*) rIndex;
		if (pItem == NULL)
			return m_empty.value;
		rIndex = (_INDEX) pItem->pNext;
		if (pKey != NULL)
			*pKey = CreateCopyKey(pItem->key);
		return pItem->value;
	}

	int GetUpperBound() const
	{
		return m_nCount;
	}

public:
	void SetEmptyItem(TYPE value, KEY key = DefKey)
	{
		DestructKey(&m_empty.key);
		m_empty.value = value;
		m_empty.key = CreateCopyKey(key);
	}

	TYPE operator [] (int nIndex) const
		{ return GetItem(nIndex); }
	TYPE operator [] (_INDEX aIndex) const
		{ return GetItem(aIndex); }
	TYPE operator [] (KEY key) const
		{ return GetItem(key); }
	TYPE& operator [] (int nIndex)
		{ return GetItemVar(nIndex); }
	TYPE& operator [] (_INDEX aIndex)
		{ return GetItemVar(aIndex); }
	TYPE& operator [] (KEY key)
		{ return GetItemVar(key); }

protected:
	int m_nCount;
	CMyKeyListItem* m_pFirst;
	CMyKeyListItem m_empty;

	CMyKeyListItem* GetLastItem() const
	{
		CMyKeyListItem* pItem = m_pFirst;
		int n = 0;
		if (pItem == NULL)
			return NULL;
		CMyKeyListItem* pNext = pItem->pNext;
		n = 1;
		while (pNext != NULL)
		{
			pItem = pNext;
			pNext = pItem->pNext;
			n++;
		}
		if (m_nCount != n)
		{
			CMyKeyList<TYPE, KEY, DefKey>* pThis = const_cast<CMyKeyList<TYPE, KEY, DefKey>*>(this);
			pThis->m_nCount = n;
		}
		return pItem;
	}

	CMyKeyListItem* GetPrevItem(CMyKeyListItem* pItem) const
	{
		CMyKeyListItem* p = m_pFirst;
		CMyKeyListItem* pPrev = NULL;
		while (p != pItem)
		{
			pPrev = p;
			p = p->pNext;
		}
		return pPrev;
	}

	CMyKeyListItem* GetItemData(int nIndex) const
	{
		CMyKeyListItem* pItem = m_pFirst;
		for (int n = 0; n < m_nCount && pItem != NULL; n++)
		{
			if (n == nIndex)
				return pItem;
			pItem = pItem->pNext;
		}
		return NULL;
	}

	virtual KEY CreateCopyKey(KEY key) const
	{
		return key;
	}

	virtual void DestructKey(KEY* pKey) const
	{
		*pKey = DefKey;
	}

	virtual BOOL CompareKey(KEY key1, KEY key2) const
	{
		return key1 == key2;
	}

	virtual CMyKeyListItem* CreateItem(TYPE value, KEY key) const
	{
		CMyKeyListItem* pItem = (CMyKeyListItem*) malloc(sizeof(CMyKeyListItem));
		memset(pItem, 0, sizeof(CMyKeyListItem));
		new (pItem) CMyKeyListItem;
		pItem->value = value;
		pItem->key = CreateCopyKey(key);
		return pItem;
	}

	virtual void DeleteItem(CMyKeyListItem* pItem) const
	{
		DestructKey(&pItem->key);
		pItem->~CMyKeyListItem();
		free((void*) pItem);
	}

public:
	~CMyKeyList()
	{
		RemoveAll();
		//Assert(m_nCount == 0);
	}
};

template <class TYPE> class CMyStringKeyListA : public CMyKeyList<TYPE, LPCSTR, NULL>
{
protected:
	virtual LPCSTR CreateCopyKey(LPCSTR key) const
	{
		return _strdup(key);
	}
	virtual void DestructKey(LPCSTR* pKey) const
	{
		free((LPSTR) *pKey);
		*pKey = NULL;
	}
	virtual BOOL CompareKey(LPCSTR key1, LPCSTR key2) const
	{
		return _stricmp(key1, key2) == 0;
	}
};

template <class TYPE> class CMyStringKeyListW : public CMyKeyList<TYPE, LPCWSTR, NULL>
{
protected:
	virtual LPCWSTR CreateCopyKey(LPCWSTR key) const
	{
		return _wcsdup(key);
	}
	virtual void DestructKey(LPCWSTR* pKey) const
	{
		free((LPWSTR) *pKey);
		*pKey = NULL;
	}
	virtual BOOL CompareKey(LPCWSTR key1, LPCWSTR key2) const
	{
		return _wcsicmp(key1, key2) == 0;
	}
};

#endif // __KEYLIST_H__
