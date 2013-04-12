#pragma once
/**
	@file
	@brief regex for cybozu::String
	@note regex version (for win) does not support icase

	@author MITSUNARI Shigeo
*/

#include <cybozu/string.hpp>

#ifdef __GNUC__
	#define CYBOZU_RE_USE_BOOST_REGEX
#endif

#ifdef CYBOZU_RE_USE_BOOST_REGEX
	#define CYBOZU_RE_STD boost
	#ifndef BOOST_REGEX_USE_CPP_LOCALE
		#define BOOST_REGEX_USE_CPP_LOCALE
	#endif
	#include <boost/regex.hpp>
#else
#if (CYBOZU_CPP_VERSION == CYBOZU_CPP_VERSION_TR1)
	#define CYBOZU_RE_STD std::tr1
#else
	#define CYBOZU_RE_STD std
#endif
	#ifdef _MSC_VER
	#ifdef _DLL_CPPLIB
		#error "use /MT or /MTd option. /MD and /MDd are not supported"
	#endif
		#define CYBOZU_STRING_USE_WIN
		#pragma warning(disable : 4018) // signed/unsigned mismatch
	#endif
	#include <regex>
#endif

#ifdef _MSC_VER
	#define CYBOZU_RE_CHAR wchar_t
	#define CYBOZU_RE(x) L##x
#else
	#define CYBOZU_RE_CHAR char
	#define CYBOZU_RE(x) x
#endif

namespace regex_local {

/* true if c in [min, max] */
inline bool in(unsigned int c, char min, char max)
{
//	  return min <= c && c <= max;
	return static_cast<unsigned int>(c - min) <= static_cast<unsigned int>(max - min);
}

} // regex_local

#ifdef CYBOZU_STRING_USE_WIN

namespace std {

template<>
class ctype<cybozu::Char> : public ctype_base {
	const std::ctype_base::mask *maskTbl_;
public:
	static locale::id id;
	ctype(size_t refs = 0) : ctype_base(refs)
	{
		static struct Custom : public std::ctype<char> {
			const mask * table() const { return std::ctype<char>::table(); }
		} custom;
		maskTbl_ = custom.table();
	}
	bool is(std::ctype_base::mask m, cybozu::Char c) const
	{
	/*
		s(48):p(1d7):c(20):u(1):l(2):a(103):d(4):p(10):x(80)
	printf("u(%x):l(%x):d(%x):p(%x):c(%x):x(%x):s(%x):p(%x):a(%x)\n"
	, std::ctype_base::upper //  0x001
	, std::ctype_base::lower //  0x002
	, std::ctype_base::digit //  0x004
	, std::ctype_base::punct //  0x010
	, std::ctype_base::cntrl //  0x020
	, std::ctype_base::xdigit // 0x080
	, std::ctype_base::space //  0x048
	, std::ctype_base::print //  0x1d7
	, std::ctype_base::alpha //  0x103
	);exit(1);
	*/
		if (c & ~0xFFU) return false;
		return (maskTbl_[(unsigned char)c] & m) != 0;
//		static const std::ctype<char>& cache = use_facet<ctype<char> >(std::locale());
//		return cache.is(m, static_cast<char>(c));
	}
	const cybozu::Char* is(const cybozu::Char* begin, const cybozu::Char* end, mask* dest) const
	{
		while (begin != end) {
			*dest++ = maskTbl_[(unsigned char)narrow(*begin)];
			begin++;
		}
		return begin;
	}
	const cybozu::Char* scan_is(std::ctype_base::mask m, const cybozu::Char* begin, const cybozu::Char* end) const
	{
		while (begin != end && !is(m, *begin)) begin++;
		return begin;
	}
	const cybozu::Char* scan_not(std::ctype_base::mask m, const cybozu::Char* begin, const cybozu::Char* end) const
	{
		while (begin != end && is(m, *begin)) begin++;
		return begin;
	}
	cybozu::Char tolower(cybozu::Char c) const
	{
		if ('A' <= c && c <= 'Z') return c - 'A' + 'a';
		return c;
	}
	const cybozu::Char* tolower(cybozu::Char* begin, const cybozu::Char* end) const
	{
		while (begin != end) {
			*begin = tolower(*begin);
			begin++;
		}
		return end;
	}
	cybozu::Char toupper(cybozu::Char c) const
	{
		if ('a' <= c && c <= 'z') return c - 'a' + 'A';
		return c;
	}
	const cybozu::Char* toupper(cybozu::Char* begin, const cybozu::Char* end) const
	{
		while (begin != end) {
			*begin = toupper(*begin);
			begin++;
		}
		return end;
	}
	cybozu::Char widen(char c) const
	{
		return c;
	}
	const char* widen(const char* begin, const char* end, cybozu::Char* to) const
	{
		while (begin != end) {
			*to++ = std::ctype<cybozu::Char>::widen(*begin++);
		}
		return end;
	}
	char narrow(cybozu::Char c, char dflt = '\0') const
	{
		if ((0 <= c && c < 256) || (c == -1)) return static_cast<char>(c);
		return dflt;
	}
	const cybozu::Char* narrow(const cybozu::Char* begin, const cybozu::Char* end, char dflt, char* to) const
	{
		while (begin != end) {
			*to++ = narrow(*begin++, dflt);
		}
		return end;
	}
	virtual bool do_is(mask m, cybozu::Char c) const
	{
		return is(m, c);
	}
	virtual const cybozu::Char *do_is(const cybozu::Char *begin, const cybozu::Char *end, mask *dest) const
	{
		return is(begin, end, dest);
	}
	virtual const cybozu::Char *do_scan_is(mask m, const cybozu::Char *begin, const cybozu::Char *end) const
	{
		return scan_is(m, begin, end);
	}
	virtual const cybozu::Char *do_scan_not(mask m, const cybozu::Char *begin, const cybozu::Char *end) const
	{
		return scan_not(m, begin, end);
	}
	virtual cybozu::Char do_tolower(cybozu::Char c) const
	{
		return tolower(c);
	}
	virtual const cybozu::Char *do_tolower(cybozu::Char *begin, const cybozu::Char *end) const
	{
		return tolower(begin, end);
	}
	virtual cybozu::Char do_toupper(cybozu::Char c) const
	{
		return toupper(c);
	}
	virtual const cybozu::Char *do_toupper(cybozu::Char *begin, const cybozu::Char *end) const
	{
		return toupper(begin, end);
	}
	virtual cybozu::Char do_widen(char c) const
	{
		return widen(c);
	}
	virtual const char *do_widen(const char *begin, const char *end, cybozu::Char *dest) const
	{
		return widen(begin, end, dest);
	}
	virtual char do_narrow(cybozu::Char c, char dflt) const
	{
		return narrow(c, dflt);
	}
	virtual const cybozu::Char *do_narrow(const cybozu::Char *begin, const cybozu::Char *end, char dflt, char *dest) const
	{
		return narrow(begin, end, dflt, dest);
	}
};

locale::id __declspec(selectany) ctype<cybozu::Char>::id;

#if defined(_DLL_CPPLIB)
__PURE_APPDOMAIN_GLOBAL std::locale::id ctype<cybozu::Char>::id;
#endif /* defined(_DLL_CPPLIB) etc. */

} // std

template<>
class CYBOZU_RE_STD::regex_traits<cybozu::Char> : public CYBOZU_RE_STD::_Regex_traits<cybozu::Char> {
public:
	int value(cybozu::Char ch, int base) const
	{
		if (base != 8 && '0' <= ch && ch <= '9' || base == 8 && '0' <= ch && ch <= '7') return ch - '0';
		if (base != 16) return -1;
		if ('a' <= ch && ch <= 'f') return ch - 'a' + 10;
		if ('A' <= ch && ch <= 'F') return ch - 'A' + 10;
		return -1;
	}
};

#define CYBOZU_RE_CHAR_CLASS_NAME(n, c) { n, sizeof(n)/sizeof(n[0]) - 1, c }

namespace cybozu { namespace regex_local {

static const cybozu::Char _alpha[] = { 'a', 'l', 'p', 'h', 'a', 0 };
static const cybozu::Char _blank[] = { 'b', 'l', 'a', 'n', 'k', 0 };
static const cybozu::Char _cntrl[] = {'c', 'n', 't', 'r', 'l', 0 };
static const cybozu::Char _d[] = {'d', 0 };
static const cybozu::Char _digit[] = {'d', 'i', 'g', 'i', 't', 0 };
static const cybozu::Char _graph[] = {'g', 'r', 'a', 'p', 'h', 0 };
static const cybozu::Char _lower[] = {'l', 'o', 'w', 'e', 'r', 0 };
static const cybozu::Char _print[] = {'p', 'r', 'i', 'n', 't', 0 };
static const cybozu::Char _punct[] = {'p', 'u', 'n', 'c', 't', 0 };
static const cybozu::Char _space[] = {'s', 'p', 'a', 'c', 'e', 0 };
static const cybozu::Char _s[] = {'s', 0 };
static const cybozu::Char _upper[] = {'u', 'p', 'p', 'e', 'r', 0 };
static const cybozu::Char _w[] = {'w', 0 };
static const cybozu::Char _xdigit[]= {'x', 'd', 'i', 'g', 'i', 't', 0 };

} }

template<>
const CYBOZU_RE_STD::_Cl_names<cybozu::Char> CYBOZU_RE_STD::_Regex_traits<cybozu::Char>::_Names[] = {
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_alpha, _Regex_traits<cybozu::Char>::_Ch_alnum),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_blank, _Regex_traits<cybozu::Char>::_Ch_blank),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_cntrl, _Regex_traits<cybozu::Char>::_Ch_cntrl),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_d, _Regex_traits<cybozu::Char>::_Ch_digit),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_digit, _Regex_traits<cybozu::Char>::_Ch_digit),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_graph, _Regex_traits<cybozu::Char>::_Ch_graph),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_lower, _Regex_traits<cybozu::Char>::_Ch_lower),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_print, _Regex_traits<cybozu::Char>::_Ch_print),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_punct, _Regex_traits<cybozu::Char>::_Ch_punct),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_space, _Regex_traits<cybozu::Char>::_Ch_space),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_s, _Regex_traits<cybozu::Char>::_Ch_space),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_upper, _Regex_traits<cybozu::Char>::_Ch_upper),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_w, (_STD ctype_base::mask)(-1)),
	CYBOZU_RE_CHAR_CLASS_NAME(cybozu::regex_local::_xdigit, _Regex_traits<cybozu::Char>::_Ch_xdigit),
	{0, 0, 0},
};

#undef CYBOZU_RE_CHAR_CLASS_NAME

#endif // CYBOZU_STRING_USE_WIN

namespace cybozu {

struct regex : public CYBOZU_RE_STD::basic_regex<Char> {
	typedef size_t size_type;
	explicit regex(){}
	explicit regex(const Char* p, flag_type f = CYBOZU_RE_STD::regex_constants::ECMAScript)
		: CYBOZU_RE_STD::basic_regex<Char>(p, f)
	{
	}
	explicit regex(const Char16* p, flag_type f = CYBOZU_RE_STD::regex_constants::ECMAScript)
		: CYBOZU_RE_STD::basic_regex<Char>(String(p).c_str(), f)
	{
	}
	explicit regex(const String& p, flag_type f = CYBOZU_RE_STD::regex_constants::ECMAScript)
		: CYBOZU_RE_STD::basic_regex<Char>(p.c_str(), f)
	{
	}
	regex(const Char* p1, const Char* p2, flag_type f = CYBOZU_RE_STD::regex_constants::ECMAScript)
		: CYBOZU_RE_STD::basic_regex<Char>(p1, p2, f)
	{
	}
	regex(const Char* p, size_type len, flag_type f)
		: CYBOZU_RE_STD::basic_regex<Char>(p, len, f)
	{
	}
	regex(const regex& that)
	  : CYBOZU_RE_STD::basic_regex<Char>(that) {}
};

class sub_match : public CYBOZU_RE_STD::sub_match<cybozu::String::const_iterator> {
	typedef CYBOZU_RE_STD::sub_match<cybozu::String::const_iterator> Base;
public:
	cybozu::String str() const {
		cybozu::String str(Base::str());
		return str;
	}
	operator cybozu::String() const { return str(); }
};

class smatch : public CYBOZU_RE_STD::match_results<cybozu::String::const_iterator> {
	typedef CYBOZU_RE_STD::match_results<cybozu::String::const_iterator> Base;
public:
	String str(int sub = 0) const { return Base::str(sub); }
	const sub_match& prefix() const { return *(const sub_match*)&(Base::prefix()); }
	const sub_match& suffix() const { return *(const sub_match*)&(Base::suffix()); }
};

struct sregex_iterator : public CYBOZU_RE_STD::regex_iterator<String::const_iterator> {
   sregex_iterator(){}
   sregex_iterator(const String::const_iterator a, const String::const_iterator b, const regex& re, CYBOZU_RE_STD::regex_constants::match_flag_type m = CYBOZU_RE_STD::regex_constants::match_default)
	  : CYBOZU_RE_STD::regex_iterator<String::const_iterator>(a, b, re, m) {}
   sregex_iterator(const sregex_iterator& that)
	  : CYBOZU_RE_STD::regex_iterator<String::const_iterator>(that) {}
#ifndef CYBOZU_STRING_USE_WIN /* return temporary address when using vc tr1? */
	const smatch& operator*()const
	{ return *static_cast<const smatch*>(&CYBOZU_RE_STD::regex_iterator<String::const_iterator>(*this).operator*()); }
	const smatch* operator->()const
	{ return static_cast<const smatch*>(CYBOZU_RE_STD::regex_iterator<String::const_iterator>(*this).operator->()); }
#endif
};

class sregex_token_iterator : public CYBOZU_RE_STD::regex_token_iterator<cybozu::String::const_iterator> {
	typedef CYBOZU_RE_STD::regex_token_iterator<cybozu::String::const_iterator> Base;
	typedef cybozu::String::const_iterator Iterator;
public:
	sregex_token_iterator() {}
	sregex_token_iterator(Iterator begin, Iterator end, const regex& re, int sub = 0, CYBOZU_RE_STD::regex_constants::match_flag_type m = CYBOZU_RE_STD::regex_constants::match_default)
		: Base(begin, end, re, sub, m)
	{
	}
	sregex_token_iterator(Iterator begin, Iterator end, const regex& re, const std::vector<int>& sub, CYBOZU_RE_STD::regex_constants::match_flag_type m = CYBOZU_RE_STD::regex_constants::match_default)
		: Base(begin, end, re, sub, m)
	{
	}
	template<size_t N>
	sregex_token_iterator(Iterator begin, Iterator end, const regex& re, const int (&sub)[N], CYBOZU_RE_STD::regex_constants::match_flag_type m = CYBOZU_RE_STD::regex_constants::match_default)
		: Base(begin, end, re, sub, m)
	{
	}
	sregex_token_iterator(const regex_token_iterator& rhs)
		: Base(rhs)
	{
	}
	const sub_match& operator*() const {
		return *(const sub_match*)&(Base::operator*());
	}
	sregex_token_iterator& operator++() {
		return *(sregex_token_iterator*)&(Base::operator++());
	}
	sregex_token_iterator operator++(int) {
		sregex_token_iterator prev = *this;
		++(*this);
		return prev;
	}
};

inline bool regex_search(const String::const_iterator begin, const String::const_iterator end, smatch& m, const regex& e, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	return CYBOZU_RE_STD::regex_search(begin, end, m, e, flags);
}

inline bool regex_search(const String& s, smatch& m, const regex& e, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	return regex_search(s.begin(), s.end(), m, e, flags);
}

inline bool regex_search(const String& s, const regex& e, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	smatch m;
	return regex_search(s, m, e, flags);
}

inline bool regex_search(const String::const_iterator begin, const String::const_iterator end, const regex& e, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	smatch m;
	return regex_search(begin, end, m, e, flags);
}

inline String regex_replace(const String::const_iterator begin, const String::const_iterator end, const regex& e, const String& fmt, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	String result;
	CYBOZU_RE_STD::regex_replace(std::back_inserter(result.get()), begin, end, e, fmt.get(), flags);
	return result;
}

inline String regex_replace(const String& s, const regex& e, const String& fmt, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	return regex_replace(s.begin(), s.end(), e, fmt, flags);
}

inline bool regex_match(const String::const_iterator begin, const String::const_iterator end, smatch& m, const regex& e, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	return CYBOZU_RE_STD::regex_match(begin, end, m, e, flags);
}

inline bool regex_match(const String& s, smatch& m, const regex& e, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	return regex_match(s.begin(), s.end(), m, e, flags);
}

inline bool regex_match(const String::const_iterator begin, const String::const_iterator end, const regex& e, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	smatch m;
	return regex_match(begin, end, m, e, flags);
}

inline bool regex_match(const String& s, const regex& e, CYBOZU_RE_STD::regex_constants::match_flag_type flags = CYBOZU_RE_STD::regex_constants::match_default)
{
	smatch m;
	return regex_match(s, m, e, flags);
}

//using CYBOZU_RE_STD::regex_token_iterator;
//typedef regex_token_iterator<cybozu::String::const_iterator> sregex_token_iterator;

} // cybozu
