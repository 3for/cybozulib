#pragma once
/**
	@file
	@brief atomic operation

	Copyright (C) 2007 Cybozu Labs, Inc., all rights reserved.
	@author MITSUNARI Shigeo
*/
#include <cybozu/inttype.hpp>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <intrin.h>
#else
#include <emmintrin.h>
#endif

namespace cybozu {

namespace atomic_util {

template<size_t S>
struct Tag {};

template<>
struct Tag<4> {
	typedef int type;

	template<class T>
	static inline T AtomicAddSub(T *p, T y)
	{
#ifdef _WIN32
		return (T)_InterlockedExchangeAdd((long*)p, (long)y);
#else
		return (T)__sync_fetch_and_add(p, y);
#endif
	}

	template<class T>
	static inline T AtomicCompareExchangeSub(T *p, T newValue, T oldValue)
	{
#ifdef _WIN32
		return (T)InterlockedCompareExchange((long*)p, (long)newValue, (long)oldValue);
#else
		return (T)__sync_val_compare_and_swap(p, oldValue, newValue);
#endif
	}

	template<class T>
	static inline T AtomicExchangeSub(T *p, T newValue)
	{
#ifdef _WIN32
		return (T)InterlockedExchange((long*)p, (long)newValue);
#else
		return (T)__sync_lock_test_and_set(p, newValue);
#endif
	}
};

template<>
struct Tag<8> {
	typedef int64_t type;

#ifdef CYBOZU_64BIT
	template<class T>
	static inline T AtomicAddSub(T *p, T y)
	{
#ifdef _WIN32
		return (T)_InterlockedExchangeAdd64((int64_t*)p, (int64_t)y);
#else
		return (T)__sync_fetch_and_add(p, y);
#endif
	}
#endif

	template<class T>
	static inline T AtomicCompareExchangeSub(T *p, T newValue, T oldValue)
	{
#ifdef _WIN32
		return (T)_InterlockedCompareExchange64((int64_t*)p, (int64_t)newValue, (int64_t)oldValue);
#else
		return (T)__sync_val_compare_and_swap(p, oldValue, newValue);
#endif
	}

#ifdef CYBOZU_64BIT
	template<class T>
	static inline T AtomicExchangeSub(T *p, T newValue)
	{
#ifdef _WIN32
		return (T)_InterlockedExchange64((int64_t*)p, (int64_t)newValue);
#else
		return (T)__sync_lock_test_and_set(p, newValue);
#endif
	}
#endif
};

} // atomic_util

/**
	atomic operation
	see http://gcc.gnu.org/onlinedocs/gcc-4.4.0/gcc/Atomic-Builtins.html
	http://msdn.microsoft.com/en-us/library/ms683504(VS.85).aspx
*/
/**
	tmp = *p;
	*p += y;
	return tmp;
*/
template<class T>
T AtomicAdd(T *p, T y)
{
	return atomic_util::Tag<sizeof(T)>::AtomicAddSub(p, y);
}

/**
	tmp = *p;
	if (*p == oldValue) *p = newValue;
	return tmp;
*/
template<class T>
T AtomicCompareExchange(T *p, T newValue, T oldValue)
{
	return atomic_util::Tag<sizeof(T)>::AtomicCompareExchangeSub(p, newValue, oldValue);
}

/**
	tmp *p;
	*p = newValue;
	return tmp;
*/
template<class T>
T AtomicExchange(T *p, T newValue)
{
	return atomic_util::Tag<sizeof(T)>::AtomicExchangeSub(p, newValue);
}

inline void mfence()
{
	_mm_mfence();
}

} // cybozu
