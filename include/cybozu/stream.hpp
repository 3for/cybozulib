#pragma once
/**
	@file
	@brief stream and line stream class

	Copyright (C) Cybozu Labs, Inc., all rights reserved.
*/
#include <string>
#include <iosfwd>
#include <cybozu/exception.hpp>
#include <cybozu/stream_fwd.hpp>

namespace cybozu {

namespace stream_local {

template <typename From, typename To>
struct is_convertible {
	typedef char yes;
	typedef int no;

	static no test(...);
	static yes test(const To*);
	static const bool value = sizeof(test((const From*)0)) == sizeof(yes);
};

template <bool b, class T = void>
struct enable_if { typedef T type; };

template <class T>
struct enable_if<false, T> {};

/* specialization for istream */
template<class InputStream>
size_t readSome_inner(InputStream& is, void *buf, size_t size, typename enable_if<is_convertible<InputStream, std::istream>::value>::type* = 0)
{
	is.read((char *)buf, size);
	const std::streamsize readSize = is.gcount();
	if (readSize < 0) throw cybozu::Exception("stream:readSome_inner:bad readSize") << size << readSize;
	return readSize;
}

template<class InputStream>
void read_inner(InputStream& is, void *buf, size_t size, typename enable_if<is_convertible<InputStream, std::istream>::value>::type* = 0)
{
	if (!is.read((char *)buf, size) || size != (size_t)is.gcount()) throw cybozu::Exception("stream:read_inner") << size;
}

/* specialization for ostream */
template<class OutputStream>
void writeSub(OutputStream& os, const void *buf, size_t size, typename enable_if<is_convertible<OutputStream, std::ostream>::value>::type* = 0)
{
	if (!os.write((const char *)buf, size)) throw cybozu::Exception("stream:writeSub") << size;
}

/* generic version for size_t readSome(void *, size_t) */
template<class InputStream>
size_t readSome_inner(InputStream& is, void *buf, size_t size, typename enable_if<!is_convertible<InputStream, std::istream>::value>::type* = 0)
{
	return is.readSome(buf, size);
}

/* generic version for void read(void *, size_t), which reads all data */
template<class InputStream>
void read_inner(InputStream& is, void *buf, size_t size, typename enable_if<!is_convertible<InputStream, std::istream>::value>::type* = 0)
{
	is.read(buf, size);
}

/* generic version for void write(const void*, size_t), which writes all data */
template<class OutputStream>
void writeSub(OutputStream& os, const void *buf, size_t size, typename enable_if<!is_convertible<OutputStream, std::ostream>::value>::type* = 0)
{
	os.write(buf, size);
}

} // stream_local

/*
	make a specializaiton of class to use new InputStream, OutputStream
*/
template<class InputStream>
struct InputStreamTag {
	static size_t readSome(InputStream& is, void *buf, size_t size)
	{
		return stream_local::readSome_inner<InputStream>(is, buf, size);
	}
	static inline void read(InputStream& is, void *buf, size_t size)
	{
		stream_local::read_inner<InputStream>(is, buf, size);
	}
};

template<class OutputStream>
struct OutputStreamTag {
	static inline void write(OutputStream& os, const void *buf, size_t size)
	{
		stream_local::writeSub<OutputStream>(os, buf, size);
	}
};

class MemoryInputStream {
	const char *p_;
	size_t size_;
public:
	MemoryInputStream(const void *p, size_t size) : p_((const char *)p), size_(size) {}
	size_t readSome(void *buf, size_t size)
	{
		if (size > size_) size = size_;
		memcpy(buf, p_, size);
		p_ += size;
		size_ -= size;
		return size;
	}
	void read(void *buf, size_t size)
	{
		if (readSome(buf, size) != size) throw cybozu::Exception("MemoryInputStream:read") << size;
	}
};

class MemoryOutputStream {
	char *p_;
	size_t size_;
public:
	MemoryOutputStream(void *p, size_t size) : p_((char *)p), size_(size) {}
	void write(const void *buf, size_t size)
	{
		if (size > size_) throw cybozu::Exception("MemoryOutputStream:write") << size << size_;
		memcpy(p_, buf, size);
		p_ += size;
		size_ -= size;
	}
};

class StringInputStream {
	const std::string& str_;
	size_t cur_;
	StringInputStream(const StringInputStream&);
	void operator=(const StringInputStream&);
public:
	explicit StringInputStream(const std::string& str) : str_(str), cur_(0) {}
	size_t readSome(void *buf, size_t size)
	{
		const size_t remainSize = str_.size() - cur_;
		if (size > remainSize) size = remainSize;
		memcpy(buf, &str_[cur_], size);
		cur_ += size;
		return size;
	}
	void read(void *buf, size_t size)
	{
		if (readSome(buf, size) != size) throw cybozu::Exception("StringInputStream:read") << size;
	}
};

class StringOutputStream {
	std::string& str_;
	StringOutputStream(const StringOutputStream&);
	void operator=(const StringOutputStream&);
public:
	explicit StringOutputStream(std::string& str) : str_(str) {}
	void write(const void *buf, size_t size)
	{
		str_.append((const char *)buf, size);
	}
};

} // cybozu
