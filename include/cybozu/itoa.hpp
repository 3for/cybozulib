#pragma once
/**
	@file
	@brief convert integer to string(ascii)

	Copyright (C) 2008 Cybozu Labs, Inc., all rights reserved.
*/
#include <limits.h>
#include <string>
#include <cybozu/inttype.hpp>
#include <cybozu/bit_operation.hpp>

namespace cybozu {

template<class T>
size_t getHexLength(T x)
{
	return x == 0 ? 1 : cybozu::bsr(x) / 4 + 1;
}
/*
	convert x to hex string with len
	@note out should have getHexLength(x) size
	out is not NUL terminated
*/
template<class T>
void itohex(char *out, size_t len, T x, bool upCase = true)
{
	static const char *hexTbl[] = {
		"0123456789abcdef",
		"0123456789ABCDEF"
	};
	const char *tbl = hexTbl[upCase];
	for (size_t i = 0; i < len; i++) {
		out[len - i - 1] = tbl[x % 16];
		x /= 16;
	}
}

namespace itoa_local {

template<typename T, typename UT, int n>
void convertFromInt(std::string& out, T x, T minusMax, const char (&minusMaxStr)[n])
{
	if (x == minusMax) {
		out.assign(minusMaxStr, minusMaxStr + n - 1);
		return;
	}
	if (x == 0) {
		out.assign(1, '0');
		return;
	}
	UT absX = x < 0 ? -x : x;
	char buf[40];
	int p = 0;
	while (absX > 0) {
		buf[p++] = '0' + (int)(absX % 10);
		absX /= 10;
	}
	if (x < 0) {
		buf[p++] = '-';
	}
	out.resize(p);
	for (int i = 0; i < p; i++) {
		out[i] = buf[p - 1 - i];
	}
}

template<typename T>
void convertFromUint(std::string& out, T x)
{
	if (x == 0) {
		out.assign(1, '0');
		return;
	}
	char buf[40];
	int p = 0;
	while (x > 0) {
		buf[p++] = '0' + (int)(x % 10);
		x /= 10;
	}
	out.resize(p);
	for (int i = 0; i < p; i++) {
		out[i] = buf[p - 1 - i];
	}
}

template<typename T>
void itohexLocal(std::string& out, T x, bool upCase, bool withZero)
{
	const size_t size = withZero ? sizeof(T) * 2 : getHexLength(x);
	out.resize(size);
	itohex(&out[0], size, x, upCase);
}

} // itoa_local

/**
	convert int to string
	@param out [out] string
	@param x [in] int
*/
inline void itoa(std::string& out, int x)
{
	itoa_local::convertFromInt<int, unsigned int>(out, x, INT_MIN, "-2147483648");
}

/**
	convert int64_t to string
	@param out [out] string
	@param x [in] int64_t
*/
inline void itoa(std::string& out, int64_t x)
{
	itoa_local::convertFromInt<int64_t, uint64_t>(out, x, LLONG_MIN, "-9223372036854775808");
}

/**
	convert unsigned int to string
	@param out [out] string
	@param x [in] unsigned int
*/
inline void itoa(std::string& out, unsigned int x)
{
	itoa_local::convertFromUint(out, x);
}

/**
	convert uint64_t to string
	@param out [out] string
	@param x [in] uint64_t
*/
inline void itoa(std::string& out, uint64_t x)
{
	itoa_local::convertFromUint(out, x);
}

/**
	convert integer to string
	@param x [in] int
*/
template<typename T>
inline std::string itoa(T x)
{
	std::string ret;
	itoa(ret, x);
	return ret;
}

inline void itohex(std::string& out, unsigned char x, bool upCase = true, bool withZero = true)
{
	itoa_local::itohexLocal(out, x, upCase, withZero);
}

inline void itohex(std::string& out, unsigned short x, bool upCase = true, bool withZero = true)
{
	itoa_local::itohexLocal(out, x, upCase, withZero);
}

inline void itohex(std::string& out, unsigned int x, bool upCase = true, bool withZero = true)
{
	itoa_local::itohexLocal(out, x, upCase, withZero);
}

inline void itohex(std::string& out, uint64_t x, bool upCase = true, bool withZero = true)
{
	itoa_local::itohexLocal(out, x, upCase, withZero);
}

template<typename T>
inline std::string itohex(T x, bool upCase = true, bool withZero = true)
{
	std::string out;
	itohex(out, x, upCase, withZero);
	return out;
}

/**
	convert integer to string with zero padding
	@param x [in] int
	@param len [in] minimum lengh of string
	@param c [in] padding character
	@note
	itoa(12, 4) == "0012"
	itoa(1234, 4) == "1234"
	itoa(12345, 4) == "12345"
	itoa(-12, 4) == "-012"
*/
template<typename T>
inline std::string itoaWithZero(T x, size_t len, char c = '0')
{
	std::string ret;
	itoa(ret, x);
	if (ret.size() < len) {
		std::string zero(len - ret.size(), c);
		if (x >= 0) {
			ret = zero + ret;
		} else {
			ret = "-" + zero + ret.substr(1);
		}
	}
	return ret;
}

} // cybozu
