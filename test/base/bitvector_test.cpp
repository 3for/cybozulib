#include <cybozu/test.hpp>
#include <cybozu/bitvector.hpp>
#include <cybozu/xorshift.hpp>

CYBOZU_TEST_AUTO(set)
{
	cybozu::BitVector cv;
	std::vector<bool> sv;
	cybozu::XorShift rg;
	const size_t size = 100;
	cv.resize(size);
	sv.resize(size);
	for (int j = 0; j < 2; j++) {
		for (size_t i = 0; i < size; i++) {
			bool b = (rg.get32() & 1) != 0;
			cv.set(i, b);
			sv[i] = b;
		}
		for (size_t i = 0; i < size; i++) {
			CYBOZU_TEST_EQUAL(cv.get(i), sv[i]);
		}
	}
}

struct StdVec : std::vector<bool> {
	template<class T>
	void append(const T*src, size_t bitLen)
	{
		for (size_t i = 0; i < bitLen; i++) {
			bool b = cybozu::GetBlockBit(src, i);
			push_back(b);
		}
	}
};

template<class T>
void verifyVec(const T& v1, const StdVec& v2)
{
	CYBOZU_TEST_EQUAL(v1.size(), v2.size());
	for (size_t i = 0; i < v1.size(); i++) {
		CYBOZU_TEST_EQUAL(v1.get(i), v2[i]);
	}
}

CYBOZU_TEST_AUTO(ShiftLeftBit)
{
	const struct {
		uint32_t x[4];
		size_t bitLen;
		size_t shift;
		uint32_t y;
		uint32_t z[4];
		uint32_t ret;
	} tbl[] = {
		{ { 0x12345678, 0, 0, 0 }, 128, 0, 0, { 0x12345678, 0, 0, 0 }, 0 },
		{ { 0x12345678, 0, 0, 0 }, 32, 1, 0, { 0x2468acf0, 0, 0, 0 }, 0 },
		{ { 0xf2345678, 0, 0, 0 }, 32, 1, 5, { 0xe468acf5, 0, 0, 0 }, 1 },
		{ { 0xffff5678, 0, 0, 0 }, 16, 1, 0, { 0xacf0, 0, 0, 0 }, 0 },
		{ { 0xffff5678, 0, 0, 0 }, 17, 1, 0, { 0x2acf0, 0, 0, 0 }, 0 },
		{ { 0x12345678, 0x9abcdef0, 0x11112222, 0xffccaaee }, 128, 19, 0x1234, { 0xb3c01234, 0xf78091a2, 0x1114d5e6, 0x57708889 }, 0x7fe65 },
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		uint32_t z[4];
		uint32_t ret = cybozu::ShiftLeftBit(z, tbl[i].x, tbl[i].bitLen, tbl[i].shift, tbl[i].y);
		const uint32_t n = cybozu::RoundupBit<uint32_t>(tbl[i].bitLen);
		CYBOZU_TEST_EQUAL_ARRAY(z, tbl[i].z, n);
		CYBOZU_TEST_EQUAL(ret, tbl[i].ret);
	}
}

CYBOZU_TEST_AUTO(ShiftRightBit)
{
	const struct {
		uint32_t x[4];
		size_t bitLen;
		size_t shift;
		uint32_t z[4];
	} tbl[] = {
		{ { 0x12345678, 0, 0, 0 }, 128, 0, { 0x12345678, 0, 0, 0 } },
		{ { 0x12345678, 0, 0, 0 }, 1, 0, { 0, 0, 0, 0 } },
		{ { 0x12345678, 0, 0, 0 }, 10, 0, { 0x278, 0, 0, 0 } },
		{ { 0x12345678, 0, 0, 0 }, 1, 1, { 0, 0, 0, 0 } },
		{ { 0x12345678, 0, 0, 0 }, 13, 5, { 0xb3, 0, 0, 0 } },
		{ { 0x12345678, 0xaaaabbbb, 0xffeebbcc, 0xfeba9874 }, 128, 1, { 0x891a2b3c, 0x55555ddd, 0x7ff75de6, 0x7f5d4c3a } },
		{ { 0x12345678, 0xaaaabbbb, 0xffeebbcc, 0xfeba9874 }, 128, 18, { 0xaeeec48d, 0xaef32aaa, 0xa61d3ffb, 0x3fae } },
		{ { 0x12345678, 0xaaaabbbb, 0xffeebbcc, 0xfeba9874 }, 97, 18, { 0xaeeec48d, 0xaef32aaa, 0x3ffb, 0 } },
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		uint32_t z[4];
		cybozu::ShiftRightBit(z, tbl[i].x, tbl[i].bitLen, tbl[i].shift);
		const uint32_t n = cybozu::RoundupBit<uint32_t>(tbl[i].bitLen);
		CYBOZU_TEST_EQUAL_ARRAY(z, tbl[i].z, n);
	}
}

CYBOZU_TEST_AUTO(append)
{
	const uint16_t src1[] = { 0x3210, 0x7654, 0xba98 };
	const uint16_t src2[] = { 0xabcd, 0xfebd, 0xffff };
	typedef cybozu::BitVectorT<uint16_t> Vec;
	{
		Vec v1;
		StdVec v2;
		v1.append(src1, 2);
		v2.append(src1, 2);
		v1.append(src2, 16);
		v2.append(src2, 16);
		verifyVec(v1, v2);
		Vec v3, v4;
		v3.append(src1, 2);
		v4.append(src2, 16);
		v3.append(v4);
		CYBOZU_TEST_ASSERT(v1 == v3);
	}
	for (size_t i = 0; i < 48; i++) {
		for (size_t j = 0; j < 48; j++) {
			Vec v1;
			StdVec v2;
			v1.append(src1, i);
			v2.append(src1, i);
			v1.append(src2, j);
			v2.append(src2, j);
			verifyVec(v1, v2);
			Vec v3, v4;
			v3.append(src1, i);
			v4.append(src2, j);
			v3.append(v4);
			CYBOZU_TEST_ASSERT(v1 == v3);
		}
	}
}

CYBOZU_TEST_AUTO(extract)
{
	const uint16_t src1[] = { 0x3210, 0x7654, 0xba98, 0xabcd, 0x98db };
	typedef cybozu::BitVectorT<uint16_t> Vec;
	Vec v;
	v.append(src1, sizeof(src1) * 8);
	for (size_t pos = 0; pos <= 33; pos++) {
		for (size_t bitLen = 0; bitLen <= 33; bitLen++) {
			uint16_t dst[3];
			v.extract(dst, pos, bitLen);
			for (size_t i = 0; i < bitLen; i++) {
				CYBOZU_TEST_EQUAL(v.get(pos + i), cybozu::GetBlockBit(dst, i));
			}
		}
		for (size_t bitLen = 0; bitLen <= 33; bitLen++) {
			Vec v2;
			v.extract(v2, pos, bitLen);
			for (size_t i = 0; i < bitLen; i++) {
				CYBOZU_TEST_EQUAL(v.get(pos + i), v2.get(i));
			}
		}
		for (size_t bitLen = 0; bitLen <= 16; bitLen++) {
			uint16_t r = v.extract(pos, bitLen);
			for (size_t i = 0; i < bitLen; i++) {
				CYBOZU_TEST_EQUAL(v.get(pos + i), cybozu::GetBlockBit(&r, i));
			}
		}
	}
}
