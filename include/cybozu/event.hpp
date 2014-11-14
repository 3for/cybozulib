#pragma once
/**
	@file
	@brief event class

	Copyright (C) 2007 Cybozu Labs, Inc., all rights reserved.
*/

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include <cybozu/exception.hpp>

namespace cybozu {

struct EventException : cybozu::Exception {
	EventException() : cybozu::Exception("event") { }
};

#ifdef _WIN32
class Event {
	HANDLE event_;
public:
	Event()
	{
		event_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (event_ == 0) {
			cybozu::EventException e;
			e << "CreateEvent";
			throw e;
		}
	}
	~Event()
	{
		::CloseHandle(event_);
	}
	void wakeup()
	{
		::SetEvent(event_);
	}
	void wait()
	{
		DWORD msec = INFINITE;
		if (WaitForSingleObject(event_, msec) != WAIT_OBJECT_0) {
			cybozu::EventException e;
			e << "wait";
			throw e;
		}
	}
};
#else
class Event {
	int pipefd_[2];
public:
	Event()
	{
		if (::pipe(pipefd_) < 0) {
			cybozu::EventException e;
			e << "pipe";
			throw e;
		}
	}
	~Event()
	{
		::close(pipefd_[0]);
		::close(pipefd_[1]);
	}
	void wakeup()
	{
		char c = 'a';
		ssize_t size = ::write(pipefd_[1], &c, 1);
		cybozu::disable_warning_unused_variable(size);
	}
	void wait()
	{
		char c;
		ssize_t size = ::read(pipefd_[0], &c, 1);
		cybozu::disable_warning_unused_variable(size);
	}
};
#endif

} // cybozu
