#pragma once
/**
	@file
	@brief bit operation
*/
#include <cybozu/inttype.hpp>

#if defined(_WIN32)
	#include <intrin.h>
#elif defined(__linux__)
	#include <x86intrin.h>
#elif defined(__GNUC__)
	#include <emmintrin.h>
#endif

namespace cybozu {

static inline int bsf(uint32_t x)
{
#if defined(_WIN32)
	unsigned long out;
	_BitScanForward(&out, x);
	return out;
#else
	return __builtin_ctz(x);
#endif
}

static inline int bsf64(uint64_t x)
{
#if defined(_WIN32)
	unsigned long out;
	_BitScanForward64(&out, x);
	return out;
#else
	return __builtin_ctzl(x);
#endif
}

static inline int bsr(uint32_t x)
{
#if defined(_WIN32)
	unsigned long out;
	_BitScanReverse(&out, x);
	return out;
#else
	return __builtin_clz(x) ^ 0x1f;
#endif
}

static inline int bsr64(uint64_t x)
{
#if defined(_WIN32)
	unsigned long out;
	_BitScanReverse64(&out, x);
	return out;
#else
	return __builtin_clzl(x) ^ 0x3f;
#endif
}

template<class T>
uint64_t makeBitMask64(T x)
{
	assert(x < 64);
	return (uint64_t(1) << x) - 1;
}
template<class T>
uint32_t popcnt(T x);

#if defined(_MSC_VER) || defined(__POPCNT__)
template<>
inline uint32_t popcnt<uint64_t>(uint64_t x)
{
	return (uint32_t)_mm_popcnt_u64(x);
}

template<>
inline uint32_t popcnt<uint32_t>(uint32_t x)
{
	return (uint32_t)_mm_popcnt_u32(x);
}
#endif

} // cybozu
