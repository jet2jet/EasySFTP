#pragma once

template <class T>
class CReferenceDelegationChild
{
public:
	CReferenceDelegationChild(T* pParent) : m_uRef(1), m_pParent(pParent)
	{
		static_assert(std::is_base_of<IUnknown, T>::value, "T is not based of IUnknown");

		if (_GetParent())
			_GetParent()->AddRef();
	}
	virtual ~CReferenceDelegationChild()
	{
	}

	ULONG AddRef()
	{
		auto u = ::InterlockedIncrement(&m_uRef);
		if (_GetParent())
			_GetParent()->AddRef();
		return u;
	}

	ULONG Release()
	{
		auto u = ::InterlockedDecrement(&m_uRef);
		if (_GetParent())
			_GetParent()->Release();
		if (!u)
			delete this;
		return u;
	}

	void DetachParent()
	{
		if (_GetParent())
		{
			ULONG c = m_uRef;
			for (ULONG u = 0; u < c; ++u)
				_GetParent()->Release();
		}
		m_pParent = NULL;
	}

	inline T* GetParent() const { return static_cast<T*>(m_pParent); }

private:
	inline IUnknown* _GetParent() const { return static_cast<IUnknown*>(m_pParent); }

	void* m_pParent;
	ULONG m_uRef;
};
