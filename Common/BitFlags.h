/*
 Copyright (C) 2025 jet (ジェット)

 BitFlags.h - declaration of BitFlags utility type
 */

#pragma once

#ifndef __cplusplus
#error This file must be used from C++
#endif

extern "C++" {

#include <type_traits>

template <typename Enum, typename BaseType = typename std::underlying_type<Enum>::type>
struct BitFlags
{
	constexpr BitFlags() = default;
	constexpr BitFlags(const BitFlags<Enum, BaseType>& flags) = default;
	constexpr BitFlags(Enum flags) : mFlags(static_cast<BaseType>(flags))
	{
		static_assert(std::is_enum<Enum>::value, "Enum must be enum type");
	}

	constexpr operator Enum() const { return static_cast<Enum>(mFlags); }
	operator Enum&() { return *reinterpret_cast<Enum*>(&mFlags); }

	BitFlags<Enum, BaseType>& operator=(const BitFlags<Enum, BaseType>& flags) = default;
	BitFlags<Enum, BaseType>& operator=(Enum flags)
	{
		mFlags = static_cast<BaseType>(flags);
		return *this;
	}
	BitFlags<Enum, BaseType>& operator|=(const BitFlags<Enum, BaseType>& flags)
	{
		mFlags |= flags.mFlags;
		return *this;
	}
	BitFlags<Enum, BaseType>& operator|=(Enum flags)
	{
		mFlags |= static_cast<BaseType>(flags);
		return *this;
	}
	BitFlags<Enum, BaseType>& operator&=(const BitFlags<Enum, BaseType>& flags)
	{
		mFlags &= flags.mFlags;
		return *this;
	}
	BitFlags<Enum, BaseType>& operator&=(Enum flags)
	{
		mFlags &= static_cast<BaseType>(flags);
		return *this;
	}
	BitFlags<Enum, BaseType>& operator&=(BaseType flags)
	{
		mFlags &= flags;
		return *this;
	}

	constexpr BitFlags<Enum, BaseType> operator~() const
	{
		return static_cast<Enum>(~mFlags);
	}
	constexpr BitFlags<Enum, BaseType> operator&(Enum bit) const
	{
		return static_cast<Enum>(mFlags & static_cast<BaseType>(bit));
	}
	constexpr BitFlags<Enum, BaseType> operator&(BaseType bit) const
	{
		return static_cast<Enum>(mFlags & static_cast<BaseType>(bit));
	}
	constexpr BitFlags<Enum, BaseType> operator|(Enum bit) const
	{
		return static_cast<Enum>(mFlags | static_cast<BaseType>(bit));
	}
	Enum* operator&()
	{
		return reinterpret_cast<Enum*>(&mFlags);
	}

	friend constexpr Enum operator~(Enum bit)
	{
		return static_cast<Enum>(~static_cast<BaseType>(bit));
	}

	BaseType mFlags;
};

#ifdef _DEBUG
namespace _BitFlags_Test_Namespace {
	enum class _Test : unsigned char { Value = 1 };
	using _TestBitFlags = BitFlags<_Test>;
	static_assert(std::is_trivially_copy_assignable<_TestBitFlags>::value, "Unexpected");
}
#endif

} // extern "C++"
