#pragma once
/**
	@file
	@brief line stream class

	Copyright (C) Cybozu Labs, Inc., all rights reserved.
*/
#include <string>
#include <memory.h>
#include <iosfwd>
#include <cybozu/file.hpp>

namespace cybozu {

namespace line_stream {

#define CYBOZU_STREAM_CRLF "\x0d\x0a"

const char CR = '\x0d';
const char LF = '\x0a';
const char CRLF[] = CYBOZU_STREAM_CRLF;

} // line_stream

/**
	construct lines from reader
	accept 0x0d 0xa and 0x0a and remove them
	next() returns one line without CRLF
	throw exception if line.size() > maxLineSize (line.size() does not include CRLF)

	Remark:next() may return the last data without CRLF

	Reader must have size_t read(char *buf, size_t size); method

	How to use this

	LinstStreamT<Reader> lineStream(reader);
	for (;;) {
		const std::string *line = lineStream.next();
		if (line == 0) {
		  // no data
		  break;
		}
	}
*/
template<class Reader, size_t maxLineSize = 1024 * 2>
class LineStreamT {
	Reader& reader_;
	char buf_[maxLineSize * 2];
	size_t bufSize_;
	const char *next_;
	bool eof_;
	std::string line_;
	LineStreamT(const LineStreamT&);
	void operator=(const LineStreamT&);
public:
	LineStreamT(Reader& reader)
		: reader_(reader)
		, bufSize_(0)
		, next_(buf_)
		, eof_(false)
	{
	}
	/**
		get line without CRLF
		@param begin [out] begin of line
		@param end [out] end of line
		@retval true if sucess
		@retval false if not data
	*/
	bool next(const char **begin, const char **end)
	{
		if (eof_) return false;
		for (;;) {
			const size_t remainSize = &buf_[bufSize_] - next_;
			if (remainSize > 0) {
				const char *endl = (const char *)memchr(next_, cybozu::line_stream::LF, remainSize);
				if (endl) {
					if (endl > next_ && endl[-1] == cybozu::line_stream::CR) {
						*end = endl - 1;
					} else {
						*end = endl;
					}
					*begin = next_;
					next_ = endl + 1;
					if ((size_t)(*end - *begin) > maxLineSize) {
						throw cybozu::Exception("LineStreamT:next:line is too long") << cybozu::exception::makeString(*begin, 10);
					}
					return true;
				}
				// move next_ to top of buf_
				for (size_t i = 0; i < remainSize; i++) {
					buf_[i] = next_[i];
				}
				next_ = buf_;
				bufSize_ = remainSize;
			} else {
				bufSize_ = 0;
				next_ = buf_;
			}
			ssize_t readSize = reader_.read(&buf_[bufSize_], sizeof(buf_) - bufSize_);
			if (readSize <= 0) {
				eof_ = true;
				if (bufSize_ == 0) return false;
				if (bufSize_ > maxLineSize) {
					throw cybozu::Exception("LineStreamT:next:no CRLF");
				}
				// take all remain buffer
				*begin= buf_;
				*end = &buf_[bufSize_];
				return true;
			}
			bufSize_ += readSize;
		}
	}
	/**
		get line
		@remark return value will be destroyed by calling next()
		you may modify returned string if not NULL
	*/
	std::string *next()
	{
		const char *begin;
		const char *end;
		if (next(&begin, &end)) {
			line_.assign(begin, end);
			return &line_;
		}
		return 0;
	}
};

} // cybozu