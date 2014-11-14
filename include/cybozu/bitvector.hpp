#pragma once
/**
	@file
	@brief bit vector
	@author MITSUNARI Shigeo(@herumi)
	@license modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <cybozu/exception.hpp>
#include <algorithm>
#include <vector>
#include <assert.h>

namespace cybozu {

template<class T>
size_t RoundupBit(size_t bitLen)
{
	const size_t unitSize = sizeof(T) * 8;
	return (bitLen + unitSize - 1) / unitSize;
}

template<class T>
T GetMaskBit(size_t bitLen)
{
	assert(bitLen < sizeof(T) * 8);
	return (T(1) << bitLen) - 1;
}

template<class T>
void SetBlockBit(T *buf, size_t bitLen)
{
	const size_t unitSize = sizeof(T) * 8;
	const size_t q = bitLen / unitSize;
	const size_t r = bitLen % unitSize;
	buf[q] |= T(1) << r;
}
template<class T>
void ResetBlockBit(T *buf, size_t bitLen)
{
	const size_t unitSize = sizeof(T) * 8;
	const size_t q = bitLen / unitSize;
	const size_t r = bitLen % unitSize;
	buf[q] &= ~(T(1) << r);
}
template<class T>
bool GetBlockBit(const T *buf, size_t bitLen)
{
	const size_t unitSize = sizeof(T) * 8;
	const size_t q = bitLen / unitSize;
	const size_t r = bitLen % unitSize;
	return (buf[q] & (T(1) << r)) != 0;
}

template<class T>
void CopyBit(T* dst, const T* src, size_t bitLen)
{
	const size_t unitSize = sizeof(T) * 8;
	const size_t q = bitLen / unitSize;
	const size_t r = bitLen % unitSize;
	for (size_t i = 0; i < q; i++) dst[i] = src[i];
	if (r == 0) return;
	dst[q] = src[q] & GetMaskBit<T>(r);
}
/*
	dst[] = (src[] << shift) | ext
	@param dst [out] dst[0..n)
	@param src [in] src[0..n)
	@param bitLen [in] length of src, dst
	@param shift [in] 0 <= shift < unitSize
	@param ext [in] or bit
*/
template<class T>
T ShiftLeftBit(T* dst, const T* src, size_t bitLen, size_t shift, T ext = 0)
{
	if (bitLen == 0) return 0;
	const size_t unitSize = sizeof(T) * 8;
	if (shift >= unitSize) {
		throw cybozu::Exception("ShiftLeftBit:large shift") << shift;
	}
	const size_t n = RoundupBit<T>(bitLen); // n >= 1 because bitLen > 0
	const size_t r = bitLen % unitSize;
	const T mask = r > 0 ? GetMaskBit<T>(r) : T(-1);
	if (shift == 0) {
		if (n == 1) {
			dst[0] = (src[0] & mask) | ext;
		} else {
			dst[n - 1] = src[n - 1] & mask;
			for (size_t i = n - 2; i > 0; i--) {
				dst[i] = src[i];
			}
			dst[0] = src[0] | ext;
		}
		return 0;
	}
	const size_t revShift = unitSize - shift;
	T prev = src[n - 1] & mask;
	const T ret = prev >> revShift;
	for (size_t i = n - 1; i > 0; i--) {
		T v = src[i - 1];
		dst[i] = (prev << shift) | (v >> revShift);
		prev = v;
	}
	dst[0] = (prev << shift) | ext;
	return ret;
}
/*
	dst[] = src[] >> shift
	@param dst [out] dst[0..n)
	@param src [in] src[0..n)
	@param bitLen [in] length of src, dst
	@param shift [in] 0 <= shift < (sizeof(T) * 8)
*/
template<class T>
void ShiftRightBit(T* dst, const T* src, size_t bitLen, size_t shift)
{
	if (bitLen == 0) return;
	const size_t unitSize = sizeof(T) * 8;
	if (shift >= unitSize) {
		throw cybozu::Exception("ShiftRightBit:bad shift") << shift;
	}
	if (shift == 0) {
		CopyBit<T>(dst, src, bitLen);
		return;
	}
	const size_t n = RoundupBit<T>(bitLen); // n >= 1 because bitLen > 0
	const size_t r = bitLen % unitSize;
	const T mask = r ? GetMaskBit<T>(r) : T(-1);
	const size_t revShift = unitSize - shift;
	if (n == 1) {
		dst[0] = (src[0] & mask) >> shift;
		return;
	}
	T prev = src[0];
	for (size_t i = 0; i < n - 2; i++) {
		T v = src[i + 1];
		dst[i] = (prev >> shift) | (v << revShift);
		prev = v;
	}
	{ // i = n - 1
		T v = src[n - 1] & mask;
		dst[n - 2] = (prev >> shift) | (v << revShift);
		prev = v;
	}
	dst[n - 1] = prev >> shift;
}

template<class T>
class BitVectorT {
	static const size_t unitSize = sizeof(T) * 8;
	size_t bitLen_;
	std::vector<T> v_;
public:
	BitVectorT() : bitLen_(0) {}
	BitVectorT(const T *buf, size_t bitLen)
	{
		init(buf, bitLen);
	}
	void init(const T *buf, size_t bitLen)
	{
		resize(bitLen);
		std::copy(buf, buf + v_.size(), &v_[0]);
	}
	void resize(size_t bitLen)
	{
		bitLen_ = bitLen;
		v_.resize(RoundupBit<T>(bitLen));
	}
	void reserve(size_t bitLen)
	{
		v_.reserve(RoundupBit<T>(bitLen));
	}
	bool get(size_t idx) const
	{
		if (idx >= bitLen_) throw cybozu::Exception("BitVectorT:get:bad idx") << idx;
		return GetBlockBit(v_.data(), idx);
	}
	void clear()
	{
		bitLen_ = 0;
		v_.clear();
	}
	void set(size_t idx, bool b)
	{
		if (b) {
			set(idx);
		} else {
			reset(idx);
		}
	}
	// set(idx, true);
	void set(size_t idx)
	{
		if (idx >= bitLen_) throw cybozu::Exception("BitVectorT:set:bad idx") << idx;
		SetBlockBit(v_.data(), idx);
	}
	// set(idx, false);
	void reset(size_t idx)
	{
		if (idx >= bitLen_) throw cybozu::Exception("BitVectorT:reset:bad idx") << idx;
		ResetBlockBit(v_.data(), idx);
	}
	size_t size() const { return bitLen_; }
	const T *getBlock() const { return &v_[0]; }
	T *getBlock() { return &v_[0]; }
	size_t getBlockSize() const { return v_.size(); }
	/*
		append src[0, bitLen)
	*/
	void append(const T* src, size_t bitLen)
	{
		if (bitLen == 0) return;
		const size_t sq = bitLen_ / unitSize;
		const size_t sr = bitLen_ % unitSize;
		resize(bitLen_ + bitLen);
		if (sr == 0) {
			CopyBit<T>(&v_[sq], src, bitLen);
			return;
		}
		T over = ShiftLeftBit<T>(&v_[sq], src, bitLen, sr, v_[sq] & GetMaskBit<T>(sr));
		if (RoundupBit<T>(bitLen + sr) > RoundupBit<T>(bitLen)) {
			v_[v_.size() - 1] = over;
		}
	}
	/*
		append bitVector
	*/
	void append(const BitVectorT<T>& v)
	{
		append(v.getBlock(), v.size());
	}
	/*
		dst[0, bitLen) = vec[pos, pos + bitLen)
	*/
	void extract(T* dst, size_t pos, size_t bitLen) const
	{
		if (bitLen == 0) return;
		if (pos + bitLen >= bitLen_) {
			throw cybozu::Exception("BitVectorT:bad range") << bitLen << pos << bitLen_;
		}
		const size_t q = pos / unitSize;
		const size_t r = pos % unitSize;
		if (r == 0) {
			CopyBit<T>(dst, &v_[q], bitLen);
			return;
		}
		ShiftRightBit<T>(dst, &v_[q], bitLen + r, r);
	}
	/*
		dst = vec[pos, pos + bitLen)
	*/
	void extract(BitVectorT<T>& dst, size_t pos, size_t bitLen) const
	{
		dst.resize(bitLen);
		extract(dst.getBlock(), pos, bitLen);
	}
	/*
		return vec[pos, pos + bitLen)
	*/
	T extract(size_t pos, size_t bitLen) const
	{
		if (bitLen == 0) return 0;
		if (bitLen > unitSize || pos + bitLen >= bitLen_) {
			throw cybozu::Exception("BitVectorT:bad range") << bitLen << pos << bitLen_;
		}
		const size_t q = pos / unitSize;
		const size_t r = pos % unitSize;
		T v;
		if (r == 0) {
			v = v_[q];
		} else {
			v = (v_[q] >> r) | v_[q + 1] << (unitSize - r);
		}
		if (bitLen == unitSize) {
			return v;
		} else {
			return v & GetMaskBit<T>(bitLen);
		}
	}
	bool operator==(const BitVectorT<T>& rhs) const { return v_ == rhs.v_; }
	bool operator!=(const BitVectorT<T>& rhs) const { return v_ != rhs.v_; }
};

typedef BitVectorT<size_t> BitVector;

} // cybozu
